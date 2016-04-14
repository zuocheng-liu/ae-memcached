#include "settings.h"
#include "config.h"
#include <stdio.h>
#include <unistd.h>

void settings_init(void) {
    settings.port = 11211;
    settings.udpport = 0;
    settings.interface.s_addr = htonl(INADDR_ANY);
    settings.maxbytes = 64*1024*1024; /* default is 64MB */
    settings.maxconns = 1024;         /* to limit connections-related memory to about 5MB */
    settings.verbose = 0;
    settings.oldest_live = 0;
    settings.evict_to_free = 1;       /* push old items out of cache when memory runs out */
    settings.socketpath = NULL;       /* by default, not using a unix socket */
    settings.managed = 0;
    settings.factor = 1.25;
    settings.chunk_size = 48;         /* space for a modest key and value */
    settings.lock_memory = 0;
    settings.daemonize = 0;
    settings.maxcore = 0;
    settings.username = NULL;
    settings.pid_file = NULL;
}

/* process arguments */
u_int32_t process_arguments(int argc, char **argv) {
    int c;
    struct in_addr addr;
    while ((c = getopt(argc, argv, "bp:s:U:m:Mc:khirvdl:u:P:f:s:")) != -1) {
        switch (c) {
            case 'U':
                settings.udpport = atoi(optarg);
                break;
            case 'b':
                settings.managed = 1;
                break;
            case 'p':
                settings.port = atoi(optarg);
                break;
            case 's':
                settings.socketpath = optarg;
                break;
            case 'm':
                settings.maxbytes = ((size_t)atoi(optarg))*1024*1024;
                break;
            case 'M':
                settings.evict_to_free = 0;
                break;
            case 'c':
                settings.maxconns = atoi(optarg);
                break;
            case 'h':
                usage();
                exit(0);
            case 'i':
                usage_license();
                exit(0);
            case 'k':
                settings.lock_memory = 1;
                break;
            case 'v':
                settings.verbose++;
                break;
            case 'l':
                if (!inet_pton(AF_INET, optarg, &addr)) {
                    fprintf(stderr, "Illegal address: %s\n", optarg);
                    return 1;
                } else {
                    settings.interface = addr;
                }
                break;
            case 'd':
                settings.daemonize = 1;
                break;
            case 'r':
                settings.maxcore = 1;
                break;
            case 'u':
                settings.username = optarg;
                break;
            case 'P':
                settings.pid_file = optarg;
                break;
            case 'f':
                settings.factor = atof(optarg);
                if (settings.factor <= 1.0) {
                    fprintf(stderr, "Factor must be greater than 1\n");
                    return 1;
                }
                break;
            case 'n':
                settings.chunk_size = atoi(optarg);
                if (settings.chunk_size == 0) {
                    fprintf(stderr, "Chunk size must be greater than 0\n");
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Illegal argument \"%c\"\n", c);
                return 1;
        }
    }
}

void usage(void) {
    printf(PACKAGE " " VERSION "\n");
    printf("-p <num>      port number to listen on\n");
    printf("-s <file>     unix socket path to listen on (disables network support)\n");
    printf("-l <ip_addr>  interface to listen on, default is INDRR_ANY\n");
    printf("-d            run as a daemon\n");
    printf("-r            maximize core file limit\n");
    printf("-u <username> assume identity of <username> (only when run as root)\n");
    printf("-m <num>      max memory to use for items in megabytes, default is 64 MB\n");
    printf("-M            return error on memory exhausted (rather than removing items)\n");
    printf("-c <num>      max simultaneous connections, default is 1024\n");
    printf("-k            lock down all paged memory\n");
    printf("-v            verbose (print errors/warnings while in event loop)\n");
    printf("-vv           very verbose (also print client commands/reponses)\n");
    printf("-h            print this help and exit\n");
    printf("-i            print memcached and libevent license\n");
    printf("-b            run a managed instanced (mnemonic: buckets)\n");
    printf("-P <file>     save PID in <file>, only used with -d option\n");
    printf("-f <factor>   chunk size growth factor, default 1.25\n");
    printf("-n <bytes>    minimum space allocated for key+value+flags, default 48\n");
    return;
}

void usage_license(void) {
    printf(PACKAGE " " VERSION "\n\n");
    printf(
    "Copyright (c) 2003, Danga Interactive, Inc. <http://www.danga.com/>\n"
    "All rights reserved.\n"
    "\n"
    "Redistribution and use in source and binary forms, with or without\n"
    "modification, are permitted provided that the following conditions are\n"
    "met:\n"
    "\n"
    "    * Redistributions of source code must retain the above copyright\n"
    "notice, this list of conditions and the following disclaimer.\n"
    "\n"
    "    * Redistributions in binary form must reproduce the above\n"
    "copyright notice, this list of conditions and the following disclaimer\n"
    "in the documentation and/or other materials provided with the\n"
    "distribution.\n"
    "\n"
    "    * Neither the name of the Danga Interactive nor the names of its\n"
    "contributors may be used to endorse or promote products derived from\n"
    "this software without specific prior written permission.\n"
    "\n"
    "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
    "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
    "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
    "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
    "OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
    "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
    "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
    "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
    "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
    "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
    "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
    "\n"
    "\n"
    "This product includes software developed by Niels Provos.\n"
    "\n"
    "[ libevent ]\n"
    "\n"
    "Copyright 2000-2003 Niels Provos <provos@citi.umich.edu>\n"
    "All rights reserved.\n"
    "\n"
    "Redistribution and use in source and binary forms, with or without\n"
    "modification, are permitted provided that the following conditions\n"
    "are met:\n"
    "1. Redistributions of source code must retain the above copyright\n"
    "   notice, this list of conditions and the following disclaimer.\n"
    "2. Redistributions in binary form must reproduce the above copyright\n"
    "   notice, this list of conditions and the following disclaimer in the\n"
    "   documentation and/or other materials provided with the distribution.\n"
    "3. All advertising materials mentioning features or use of this software\n"
    "   must display the following acknowledgement:\n"
    "      This product includes software developed by Niels Provos.\n"
    "4. The name of the author may not be used to endorse or promote products\n"
    "   derived from this software without specific prior written permission.\n"
    "\n"
    "THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
    "IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n"
    "OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n"
    "IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
    "INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n"
    "NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
    "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
    "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
    "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF\n"
    "THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
    );

    return;
}

