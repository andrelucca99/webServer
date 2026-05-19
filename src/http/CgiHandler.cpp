/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 00:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/09 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"
#include "../includes/HttpError.hpp"
#include <cctype>
#include <cstring>
#include <map>
#include <sstream>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CGI_TIMEOUT_MS 5000
#define CGI_READ_BUF   4096

static std::string headerToEnvKey(const std::string& name) {
    std::string out = "HTTP_";
    for (size_t i = 0; i < name.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(name[i]);
        out += (c == '-') ? '_' : static_cast<char>(std::toupper(c));
    }
    return out;
}

static std::string itos(long long n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

static char* dupStr(const std::string& s) {
    char* out = new char[s.size() + 1];
    std::memcpy(out, s.c_str(), s.size() + 1);
    return out;
}

static char** mapToEnvp(const std::map<std::string, std::string>& env) {
    char** envp = new char*[env.size() + 1];
    size_t i = 0;
    typedef std::map<std::string, std::string>::const_iterator It;
    for (It it = env.begin(); it != env.end(); ++it, ++i)
        envp[i] = dupStr(it->first + "=" + it->second);
    envp[i] = NULL;
    return envp;
}

static char** buildArgv(const std::string& interpreter, const std::string& script) {
    char** argv = new char*[3];
    argv[0] = dupStr(interpreter);
    argv[1] = dupStr(script);
    argv[2] = NULL;
    return argv;
}

static void freeStrArray(char** arr) {
    if (!arr)
        return;
    for (size_t i = 0; arr[i] != NULL; ++i)
        delete[] arr[i];
    delete[] arr;
}

CgiHandler::CgiHandler(const HttpRequest&  request,
                       const RouteConfig&  route,
                       const ServerConfig& server,
                       const std::string&  scriptPath,
                       const std::string&  interpreter)
    : _request(request),
      _route(route),
      _server(server),
      _scriptPath(scriptPath),
      _interpreter(interpreter) {}

CgiHandler::~CgiHandler() {}

std::string CgiHandler::matchCgi(const RouteConfig& route, const std::string& path) {
    typedef std::map<std::string, std::string>::const_iterator It;
    for (It it = route.cgi_extensions.begin(); it != route.cgi_extensions.end(); ++it) {
        const std::string& ext = it->first;
        if (ext.empty() || ext.size() > path.size())
            continue;
        size_t pos = path.rfind(ext);
        if (pos == std::string::npos)
            continue;
        size_t after = pos + ext.size();
        if (after == path.size() || path[after] == '/')
            return it->second;
    }
    return "";
}

std::map<std::string, std::string> CgiHandler::_buildEnv() const {
    std::map<std::string, std::string> env;

    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"]   = _request.http_version.empty() ? "HTTP/1.1"
                                                             : _request.http_version;
    env["SERVER_NAME"]       = _server.host;
    env["SERVER_PORT"]       = itos(_server.port);
    env["REQUEST_METHOD"]    = _request.method;
    env["SCRIPT_NAME"]       = _request.path;
    env["SCRIPT_FILENAME"]   = _scriptPath;
    env["PATH_INFO"]         = _request.path;
    env["QUERY_STRING"]      = _request.query_string;
    env["REDIRECT_STATUS"]   = "200";

    if (!_request.body.empty())
        env["CONTENT_LENGTH"] = itos(static_cast<long long>(_request.body.size()));

    typedef std::map<std::string, std::string>::const_iterator It;
    It ct = _request.headers.find("content-type");
    if (ct != _request.headers.end())
        env["CONTENT_TYPE"] = ct->second;

    for (It it = _request.headers.begin(); it != _request.headers.end(); ++it) {
        env[headerToEnvKey(it->first)] = it->second;
    }

    return env;
}

static std::string toLowerStr(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
        out += static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
    return out;
}

static std::string trimWs(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t'))
        ++b;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' ||
                     s[e - 1] == '\r' || s[e - 1] == '\n'))
        --e;
    return s.substr(b, e - b);
}

bool CgiHandler::_parseOutput(const std::string& raw, HttpResponse& res) const {
    size_t sep_len = 0;
    size_t sep = raw.find("\r\n\r\n");
    if (sep != std::string::npos) {
        sep_len = 4;
    } else {
        sep = raw.find("\n\n");
        if (sep != std::string::npos)
            sep_len = 2;
    }
    if (sep == std::string::npos)
        return false;

    std::string headers_blob = raw.substr(0, sep);
    std::string body         = raw.substr(sep + sep_len);

    res.status      = 200;
    res.contentType = "text/html";
    res.location    = "";

    size_t pos = 0;
    while (pos < headers_blob.size()) {
        size_t eol = headers_blob.find('\n', pos);
        std::string line = (eol == std::string::npos)
                              ? headers_blob.substr(pos)
                              : headers_blob.substr(pos, eol - pos);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1, 1);

        if (!line.empty()) {
            size_t colon = line.find(':');
            if (colon == std::string::npos)
                return false;
            std::string name  = toLowerStr(trimWs(line.substr(0, colon)));
            std::string value = trimWs(line.substr(colon + 1));

            if (name == "status") {
                std::istringstream iss(value);
                int code = 0;
                iss >> code;
                if (code > 0)
                    res.status = code;
            } else if (name == "content-type") {
                res.contentType = value;
            } else if (name == "location") {
                res.location = value;
                if (res.status == 200)
                    res.status = 302;
            } else if (name != "content-length" && name != "connection") {
                // preserva o nome original (case do CGI) para o cliente
                res.headers[trimWs(line.substr(0, colon))] = value;
            }
        }

        if (eol == std::string::npos)
            break;
        pos = eol + 1;
    }

    res.body = body;
    return true;
}

HttpResponse CgiHandler::_errorResponse(int status) const {
    HttpResponse res;
    res.status      = status;
    res.contentType = "text/html";
    res.body        = httpErrorBody(status, _server);
    return res;
}

HttpResponse CgiHandler::execute() {
    signal(SIGPIPE, SIG_IGN);

    std::map<std::string, std::string> env = _buildEnv();
    char** envp = mapToEnvp(env);
    char** argv = buildArgv(_interpreter, _scriptPath);

    int in_pipe[2];
    int out_pipe[2];

    if (pipe(in_pipe) < 0) {
        freeStrArray(envp); freeStrArray(argv);
        return _errorResponse(500);
    }
    if (pipe(out_pipe) < 0) {
        close(in_pipe[0]); close(in_pipe[1]);
        freeStrArray(envp); freeStrArray(argv);
        return _errorResponse(500);
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(in_pipe[0]); close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);
        freeStrArray(envp); freeStrArray(argv);
        return _errorResponse(500);
    }

    if (pid == 0) {
        // child
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        close(in_pipe[0]);  close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);

        std::string dir;
        std::string base;
        size_t slash = _scriptPath.find_last_of('/');
        if (slash != std::string::npos) {
            dir  = _scriptPath.substr(0, slash);
            base = _scriptPath.substr(slash + 1);
        } else {
            dir  = ".";
            base = _scriptPath;
        }
        if (chdir(dir.c_str()) != 0)
            _exit(1);

        // depois do chdir, argv[1] precisa ser relativo ao novo cwd (basename)
        delete[] argv[1];
        argv[1] = dupStr(base);

        execve(_interpreter.c_str(), argv, envp);
        _exit(1);
    }

    // parent
    close(in_pipe[0]);
    close(out_pipe[1]);

    // Subject: nunca chamar read/write sem passar por poll(). Marcamos
    // ambos os fds como nao-bloqueantes e fazemos write/read sempre apos
    // poll() acordar com POLLOUT/POLLIN.
    fcntl(in_pipe[1],  F_SETFL, O_NONBLOCK);
    fcntl(out_pipe[0], F_SETFL, O_NONBLOCK);

    const char* body_ptr  = _request.body.data();
    size_t      body_left = _request.body.size();
    bool        in_open   = true;  // stdin do filho ainda aberto
    if (body_left == 0) {
        close(in_pipe[1]);
        in_open = false;
    }

    std::string output;
    bool timed_out = false;
    struct pollfd pfds[2];

    while (true) {
        nfds_t nf = 0;
        int idx_out = -1;
        int idx_in  = -1;

        pfds[nf].fd      = out_pipe[0];
        pfds[nf].events  = POLLIN;
        pfds[nf].revents = 0;
        idx_out = nf;
        nf++;

        if (in_open) {
            pfds[nf].fd      = in_pipe[1];
            pfds[nf].events  = POLLOUT;
            pfds[nf].revents = 0;
            idx_in = nf;
            nf++;
        }

        int pr = poll(pfds, nf, CGI_TIMEOUT_MS);
        if (pr < 0) break;
        if (pr == 0) { timed_out = true; break; }

        // Escreve body no stdin do filho quando POLLOUT disponivel.
        if (in_open && idx_in >= 0 && (pfds[idx_in].revents & POLLOUT)) {
            ssize_t n = write(in_pipe[1], body_ptr, body_left);
            if (n <= 0) {
                close(in_pipe[1]);
                in_open = false;
            } else {
                body_ptr  += n;
                body_left -= static_cast<size_t>(n);
                if (body_left == 0) {
                    close(in_pipe[1]);
                    in_open = false;
                }
            }
        }

        // Le stdout do filho quando POLLIN disponivel.
        bool out_hup = false;
        if (pfds[idx_out].revents & POLLIN) {
            char buf[CGI_READ_BUF];
            ssize_t n = read(out_pipe[0], buf, sizeof(buf));
            if (n <= 0) {
                out_hup = true;
            } else {
                output.append(buf, static_cast<size_t>(n));
            }
        } else if (pfds[idx_out].revents & POLLHUP) {
            out_hup = true;
        }

        if (out_hup)
            break;
    }
    if (in_open)
        close(in_pipe[1]);
    close(out_pipe[0]);

    if (timed_out)
        kill(pid, SIGKILL);

    int status = 0;
    waitpid(pid, &status, 0);

    freeStrArray(envp);
    freeStrArray(argv);

    if (timed_out)
        return _errorResponse(504);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return _errorResponse(502);
    if (WIFSIGNALED(status))
        return _errorResponse(502);

    HttpResponse res;
    if (!_parseOutput(output, res))
        return _errorResponse(502);
    return res;
}
