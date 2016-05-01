#include "Parse_TCP.h"
#include <exception>
#include <string>

pthread_mutex_t prod [NUMLINKS];
pthread_mutex_t cons [NUMLINKS];
pthread_cond_t prod_cond [NUMLINKS];
pthread_cond_t cons_cond [NUMLINKS];
pthread_t thread_id [NUMCONSUMERS + NUMPRODUCERS], sig_wait_id;
int spaces [NUMLINKS];
int deadlock, link_no;
char * base_URL;
char * sub_domain; //consumer

std::map<ctmbstr, std::list<ctmbstr> > hash_map_;

/*****************************************************************/
/* SIGNAL WAITER, PRODUCER AND CONSUMER THREAD FUNCTIONS         */
/*****************************************************************/
static void *sig_waiter(void *arg);
static void sig_handler(int);
static void *parse_domain(void *);
static void* spider_web(void* d_link);

static size_t write_callback(char* ptr, size_t size, size_t nmemb, TidyBuffer *out) {
    size_t n = size * nmemb;
    tidyBufAppend(out, ptr, n);

    return n;
}

//default

Parse_TCP::Parse_TCP() {

}

Parse_TCP::Parse_TCP(char *argv) {
    int nsigs;
    struct sigaction new_act;
    std::string init = "";
    base_URL = argv;
    sub_domain = (char *) init.c_str();

    sigset_t all_signals;
    int sigs[] = {SIGBUS, SIGSEGV, SIGFPE};

    pthread_attr_t thread_attr; //Stack size , What cpu to run on, 
    // struct sched_param sched_struct;


    /**********************************************************************/
    /* GENERAL PTHREAD MUTEX AND CONDITION INIT AND GLOBAL INIT           */
    /**********************************************************************/

    for (int i = 0; i < NUMLINKS; i++) {
        pthread_mutex_init(&prod [i], NULL);
        pthread_mutex_init(&cons [i], NULL);
        pthread_cond_init(&prod_cond [i], NULL); //initialize before use. ALWAYS 
        pthread_cond_init(&cons_cond [i], NULL);
        spaces [i] = NUMSLOTS;
        link_no = 0;
    }


    /**********************************************************************/
    /* SETUP FOR MANAGING THE SIGTERM SIGNAL, BLOCK ALL SIGNALS           */
    /**********************************************************************/

    sigfillset(&all_signals);
    nsigs = sizeof ( sigs) / sizeof ( int);
    for (int i = 0; i < nsigs; i++)
        sigdelset(&all_signals, sigs [i]);
    sigprocmask(SIG_BLOCK, &all_signals, NULL);

    sigfillset(&all_signals);
    for (int i = 0; i < nsigs; i++) {
        new_act.sa_handler = sig_handler;
        new_act.sa_mask = all_signals;
        new_act.sa_flags = 0;
        if (sigaction(sigs[i], &new_act, NULL) == -1) {
            perror("can't set signals: ");
            exit(1);
        }
    }

    printf("just before threads created\n");


    /*********************************************************************/
    /* CREATE SIGNAL HANDLER THREAD, PRODUCER AND CONSUMERS              */
    /*********************************************************************/

    if (pthread_create(&sig_wait_id, NULL,
            sig_waiter, NULL) != 0) {

        printf("pthread_create failed ");
        exit(3);
    }

    pthread_attr_init(&thread_attr);
    pthread_attr_setinheritsched(&thread_attr,
            PTHREAD_INHERIT_SCHED); //inherit from parent (which is default)

#ifdef  GLOBAL
    sched_struct.sched_priority = sched_get_priority_max(SCHED_OTHER);
    pthread_attr_setinheritsched(&thread_attr,
            PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
    pthread_attr_setschedparam(&thread_attr, &sched_struct);
    pthread_attr_setscope(&thread_attr,
            PTHREAD_SCOPE_SYSTEM); //all available cpu if this code is available
#endif

    for (int i = 0; i < NUMCONSUMERS; i++) { //consumer threads
        if (pthread_create(&thread_id [i], NULL,
                spider_web, (void *) sub_domain) != 0) {
            printf("pthread_create failed");
            exit(3);
        }
    }
    // for (int i = 0; i < NUMPRODUCERS; i++) { // producer threads
    if (pthread_create(&thread_id [NUMCONSUMERS + NUMPRODUCERS], NULL,
            parse_domain, (void *) base_URL) != 0) {
        printf("pthread_create failed");
        exit(3);
    }
    //}

    printf("just after threads created\n");

    /*********************************************************************/
    /* WAIT FOR ALL CONSUMERS TO FINISH, SIGNAL WAITER WILL              */
    /* NOT FINISH UNLESS A SIGTERM ARRIVES AND WILL THEN EXIT            */
    /* THE ENTIRE PROCESS....OTHERWISE MAIN THREAD WILL EXIT             */
    /* THE PROCESS WHEN ALL CONSUMERS ARE FINISHED                       */
    /*********************************************************************/
    for (int i = 0; i < NUMCONSUMERS + NUMPRODUCERS; i++) {

        pthread_join(thread_id [i], NULL);
        printf("%d\n",i);
        // write(1, "*", 1);
    }

}

//consume links
//spider_web

static void* spider_web(void* d_link) {
    int j;

    char * link = NULL;

    unsigned short xsub [3];
    struct timeval randtime;

    gettimeofday(&randtime, (struct timezone *) 0);
    xsub [0] = (ushort) randtime.tv_usec;
    xsub [1] = (ushort) (randtime.tv_usec >> 16);
    xsub [2] = (long) (pthread_self());

    link = (char *) d_link;

    j = nrand48(xsub) & 3;

    pthread_mutex_lock(&cons [j]);

    for (int i = 0; i < 1000; i++) {

        while (hash_map_.empty()) {
            pthread_cond_wait(&cons_cond [j], &cons [j]);
        }

        for (std::map<ctmbstr, std::list < ctmbstr>>::iterator it = hash_map_.begin(); it != hash_map_.end(); ++it) {

            if (strncmp(it->first, link, strlen(link)) == 0) {

                //found duplicate

            } else {

                pthread_mutex_lock(&prod[j]);
                link_no++;
                *base_URL = *link;
                pthread_mutex_unlock(&prod[j]);
            }
        }

        pthread_mutex_unlock(&cons[j]);
        
        pthread_cond_signal(&prod_cond[j]); //unblock one prod thread waiting

    }
    usleep(100);
    return NULL;
}

//CURL web site
//producer helper function.

void Parse_TCP::crawl_page(TidyDoc doc, TidyNode tnod, int indent) {

    TidyNode child;

    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child)) {

        ctmbstr name = tidyNodeGetName(child);

        if (name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;

            //(strncmp(name, "a", 1) == 0) ? printf("Link: ") : printf("Not link: ");

            printf("%*.*s%s ", indent, indent, "<", name);

            /* walk the attribute list */
            for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {

                printf("%s", tidyAttrName(attr));
                tidyAttrValue(attr) ? printf("=\"%s\" ",
                        tidyAttrValue(attr)) : printf(" ");


                //but how do I uses the keys??

                if (strncmp(tidyAttrName(attr), "href", 4) == 0) {
                    //
                    const char *link = tidyAttrValue(attr); //<a href="/home"/>

                    if (link != NULL) {

                        if ((strlen(link)) < 40) { //short links

                            if (hash_map_.empty()) {

                                link_no++;

                                hash_map_[base_URL].push_back("|"); //domain ie. first link

                            } else {

                                //this should be a new thread running this
                                if (strncmp(link, "www.", 4) == 0 or strncmp(link, "http", 4) == 0 or strncmp(link, "https", 5) == 0) {

                                    sub_domain = (char *) link;

                                } else {

                                    hash_map_["home"].push_back(link);
                                }

                            }
                        }

                    }
                }


            }
            printf(">\n");
        } else {

            /* if it doesn't have a name, then it's probably text, cdata, etc... */
            TidyBuffer buf;
            tidyBufInit(&buf);
            tidyNodeGetText(doc, child, &buf);
            printf("%*.*s\n", indent, indent, buf.bp ? (char *) buf.bp : "");
            tidyBufFree(&buf);
        }
        crawl_page(doc, child, indent + 4); /* recursive */
    }
}

//CURL web site
// produce function

static void * parse_domain(void *argv) {
    int j, err = 0;
    TidyBuffer tidy_buffer = {0};
    TidyBuffer tidy_errbuffer = {0};

    Parse_TCP *ptr = new Parse_TCP;

    TidyDoc doc = tidyCreate();

    CURL * curl = curl_easy_init();

    char curl_errbuf[CURL_ERROR_SIZE];

    unsigned short xsub [3];
    struct timeval randtime;

    gettimeofday(&randtime, (struct timezone *) 0);
    xsub [0] = (ushort) randtime.tv_usec;
    xsub [1] = (ushort) (randtime.tv_usec >> 16);
    xsub [2] = (long) (pthread_self());

    while (true) {

        j = nrand48(xsub) & 3;

        tidyOptSetBool(doc, TidyForceOutput, yes); /* try harder */
        tidyOptSetInt(doc, TidyWrapLen, 4096);
        tidySetErrorBuffer(doc, &tidy_errbuffer);
        tidyBufInit(&tidy_buffer);

        curl_easy_setopt(curl, CURLOPT_URL, (char *) base_URL);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tidy_buffer);

        err = curl_easy_perform(curl);

        pthread_mutex_lock(&cons[j]);

        if (link_no > 1000) {
            //std::cout<<"links "<< hash_map_.size()<<"\n";
            //ptr->print_item(std::cout);

            pthread_mutex_unlock(&cons[j]);

            break;
        }

        pthread_mutex_unlock(&cons[j]);
        pthread_cond_signal(&cons_cond[j]); //unblock one cons thread waiting

        //pthread_mutex_unlock(&prod [j]); //done producing

        //consuming code goes here

        if (!err) {

            err = tidyParseBuffer(doc, &tidy_buffer); /* parse the input */

            if (err >= 0) {
                err = tidyCleanAndRepair(doc); /* fix any problems */
                if (err >= 0) {
                    err = tidyRunDiagnostics(doc); /* load tidy error buffer */
                    if (err >= 0) {

                        pthread_mutex_lock(&cons[j]);

                        ptr->crawl_page(doc, tidyGetRoot(doc), 0); /* walk the tree */

                        pthread_mutex_unlock(&cons[j]);
                        pthread_cond_signal(&cons_cond[j]);
                        // fprintf(stderr, "%s\n", tidy_errbuffer.bp); /* show errors */
                    }
                }
            }
        }
        usleep(100);
    }
    return NULL;
}

void Parse_TCP::print_item(std::ostream & stream) {

    for (std::map<ctmbstr, std::list < ctmbstr>>::iterator it = hash_map_.begin(); it != hash_map_.end(); ++it) {

        stream << it->first << "\n";

    }

}


/***********************************************************/
/* PTHREAD ASYNCH SIGNAL HANDLER ROUTINE...                */

/***********************************************************/

static void * sig_waiter(void *arg) {
    sigset_t sigterm_signal;
    int signo;

    sigemptyset(&sigterm_signal);
    sigaddset(&sigterm_signal, SIGTERM);
    sigaddset(&sigterm_signal, SIGINT);

    if (sigwait(&sigterm_signal, & signo) != 0) {
        printf("\n  sigwait ( ) failed, exiting \n");
        exit(2);
    }
    printf("process going down on SIGNAL (number %d)\n\n", signo);
    exit(1);

    return NULL;
}


/**********************************************************/
/* PTHREAD SYNCH SIGNAL HANDLER ROUTINE...                */

/**********************************************************/

static void sig_handler(int sig) {
    pthread_t signaled_thread_id;
    int thread_index = 0;

    signaled_thread_id = pthread_self();
    for (int i = 0; i < (NUMCONSUMERS + 1); i++) {
        if (signaled_thread_id == thread_id [i]) {
            thread_index = i;
            break;
        }
    }
    printf("\nThread %d took signal # %d, PROCESS HALT\n",
            thread_index, sig);
    exit(1);
}

