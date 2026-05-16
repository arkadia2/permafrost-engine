/*
 *  This file is part of Permafrost Engine. 
 *  Copyright (C) 2019-2023 Eduard Permyakov 
 *
 *  Permafrost Engine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Permafrost Engine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Linking this software statically or dynamically with other modules is making 
 *  a combined work based on this software. Thus, the terms and conditions of 
 *  the GNU General Public License cover the whole combination. 
 *  
 *  As a special exception, the copyright holders of Permafrost Engine give 
 *  you permission to link Permafrost Engine with independent modules to produce 
 *  an executable, regardless of the license terms of these independent 
 *  modules, and to copy and distribute the resulting executable under 
 *  terms of your choice, provided that you also meet, for each linked 
 *  independent module, the terms and conditions of the license of that 
 *  module. An independent module is a module which is not derived from 
 *  or based on Permafrost Engine. If you modify Permafrost Engine, you may 
 *  extend this exception to your version of Permafrost Engine, but you are not 
 *  obliged to do so. If you do not wish to do so, delete this exception 
 *  statement from your version.
 *
 */

#include "log.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_thread.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <fcntl.h>
#include <direct.h>
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#define read(fd, buf, len) _read(fd, buf, len)
#define write(fd, buf, len) _write(fd, buf, len)
#define dup _dup
#define dup2 _dup2
#define close _close
#else
#include <unistd.h>
#endif

static FILE *s_log_file = NULL;
static FILE *s_orig_stdout = NULL;
static FILE *s_orig_stderr = NULL;
static int  s_log_level = LOG_LEVEL_DEBUG;
static int  s_pipe_fds[2] = {-1, -1};
static SDL_Thread *s_log_thread = NULL;
static volatile int s_log_thread_running = 0;

static int log_forward_thread(void *arg)
{
    char buffer[4096];
    ssize_t n;
    
    while(s_log_thread_running && (n = read(s_pipe_fds[0], buffer, sizeof(buffer))) > 0) {
        if(s_orig_stdout) {
            fwrite(buffer, 1, n, s_orig_stdout);
            fflush(s_orig_stdout);
        }
        if(s_log_file) {
            fwrite(buffer, 1, n, s_log_file);
            fflush(s_log_file);
        }
    }
    return 0;
}

static const char *log_level_str(int level)
{
    switch(level) {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Log_Init(const char *logfile)
{
    s_orig_stdout = fdopen(dup(fileno(stdout)), "w");
    s_orig_stderr = fdopen(dup(fileno(stderr)), "w");
    
    if(logfile && strlen(logfile) > 0) {
        #ifdef _WIN32
        _mkdir("log");
        #else
        mkdir("log", 0755);
        #endif
        
        char log_path[512];
        snprintf(log_path, sizeof(log_path), "log/%s", logfile);
        s_log_file = fopen(log_path, "w");
        if(!s_log_file) {
            fprintf(s_orig_stderr, "WARNING: Failed to open log file '%s'\n", log_path);
        }
    }
    
    if(pipe(s_pipe_fds) == 0) {
        dup2(s_pipe_fds[1], fileno(stdout));
        dup2(s_pipe_fds[1], fileno(stderr));
        close(s_pipe_fds[1]);
        
        s_log_thread_running = 1;
        s_log_thread = SDL_CreateThread(log_forward_thread, "log_forward", NULL);
        if(!s_log_thread) {
            fprintf(s_orig_stderr, "WARNING: Failed to create log forward thread\n");
        }
    }
}

void Log_Shutdown(void)
{
    s_log_thread_running = 0;
    
    if(s_log_thread) {
        SDL_WaitThread(s_log_thread, NULL);
        s_log_thread = NULL;
    }
    
    if(s_pipe_fds[0] >= 0) {
        close(s_pipe_fds[0]);
        s_pipe_fds[0] = -1;
    }
    
    if(s_log_file) {
        fclose(s_log_file);
        s_log_file = NULL;
    }
    
    if(s_orig_stdout) {
        fclose(s_orig_stdout);
        s_orig_stdout = NULL;
    }
    
    if(s_orig_stderr) {
        fclose(s_orig_stderr);
        s_orig_stderr = NULL;
    }
}

void Log_SetLevel(int level)
{
    if(level >= LOG_LEVEL_DEBUG && level <= LOG_LEVEL_ERROR) {
        s_log_level = level;
    }
}

static void log_write(int level, const char *format, va_list args)
{
    if(level < s_log_level)
        return;

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm);

    char fmt_buf[512];
    snprintf(fmt_buf, sizeof(fmt_buf), "[%s] %s: %s\n", 
             time_str, log_level_str(level), format);

    FILE *out = (level == LOG_LEVEL_ERROR) ? stderr : stdout;
    vfprintf(out, fmt_buf, args);
    fflush(out);
}

void Log_Debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void Log_Info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void Log_Warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

void Log_Error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}
