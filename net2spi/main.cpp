#include <iostream>
#include "mcontrol.h"
#include <signal.h>
#include "mspi.h"
#include "mnet.h"
#include "mconfig.h"
//#include "mpanel.h"

MControl *control;
MSpi	 *spi;
MNet     *g_net=NULL;
MNet     *net;
// MPanel 	 *panel;

int sigexit=0;
void sig_handler(int signo) {
    switch (signo) {
        case SIGKILL:
        case SIGINT:
            sigexit=1;
        break;
        case SIGPIPE:
        {
          printf("signal Broken pipe\n");
         if(net!=NULL)
           net->stop();
         if(control!=NULL)
           control->stop();

        }
          break;
    }
    return;
}

int main(int argc, char **argv) {
    signal(SIGKILL, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGPIPE, sig_handler);
    signal(SIGUSR2, SIG_IGN);

    pthread_mutex_t  lock_spi;

    __pid_t pid=getpid();

    printf("process id is %d\r\n",pid);
    int fd_pid=open("/var/volatile/net2spi.pid",O_RDWR|O_CREAT);
    if(fd_pid>=0)
    {
        char buff[64];
        memset(buff,0,64);
        sprintf(buff,"%d\n",pid);
        write(fd_pid,buff,strlen(buff));
        close(fd_pid);
    }


    //int ret=pthread_mutex_destroy(&lock_spi);
    //printf("lock ret %d\r\n",ret);
    //printf("%d,%d\r\n",sizeof(pack_rw_info),sizeof(data_head_t));
    //sleep(100);
    spi=NULL;
    control=NULL;
    net=NULL;
    g_net=NULL;
    spi=new MSpi();
    if(spi==NULL)
    {
        printf("err:new spi\r\n");
    }

    if(spi->initialize())
    {
        goto main_err1;
    }

// 	panel=new MPanel();
// 	if(panel==NULL)
// 	{
// 		goto main_err1;
// 	}

// 	if(panel->initialize(spi))
// 	{
// 		goto main_err2;
// 	}
//
// 	panel->start();
    while(!sigexit)
    {
        //printf("start");
        signal(SIGUSR2, SIG_IGN);
        control=new MControl();
        if(control==NULL)
        {
            printf("err:new control\r\n");
            continue;
        }

        if(control->initialize(spi))
        {
            goto main_err3;
        }


        net = new MNet();
        if(net==NULL)
        {
            goto main_err3;
        }
        if(argc>=2)
        {
            if(net->initialize(spi,control,argv[1])!=0)
            {
                goto main_err4;
            }
        }
        else
        {
            int fd_ip=open("/mooncell/etc/srv.cfg",O_RDONLY);

            if(fd_ip<=0)
            {
                //printf("con't open /mooncell/etc/srv.cfg");
                if(net->initialize(spi,control,(char*)NET_SERVER_ADDR)!=0)
                {
                    goto main_err4;
                }
            }
            else{
                char ip_buff[64];
                memset(ip_buff,0,64);
                int size=lseek(fd_ip,0,SEEK_END);
                size=size>64?64:size;
                lseek(fd_ip,0,SEEK_SET);
                read(fd_ip,ip_buff,size);
                char *p=ip_buff;
                close(fd_ip);

                for(int i=0;i<size;i++)
                {

                    if((*p>='0'&& *p<='9')||*p=='.')
                    {
                        p++;
                        continue;
                    }
                    else
                    {
                        *p=0;
                        //*(p+1)=0;
                        break;
                    }
                }
                if(ip_buff[0]==0)
                {
                    //printf("con't open /mooncell/etc/srv.cfg");
                    if(net->initialize(spi,control,(char*)NET_SERVER_ADDR)!=0)
                    {
                        goto main_err4;
                    }
                }
                else{
                    //printf(ip_buff);
                    if(net->initialize(spi,control,ip_buff)!=0)
                    {
                        goto main_err4;
                    }
                }


            }
        }

        control->start();
        net->start();
        g_net=net;
        //printf("net2spi initialize ok!\r\n");

        net->join();//等待退出。
        control->stop();//等待退出。
        control->join();//等待退出。
        sleep(3);



main_err4:
        delete net;
        net=NULL;
        g_net=NULL;

main_err3:
        delete control;

main_err5:
        control=NULL;

    }

// 		panel->stop();
// 		panel->join();
//
// main_err2:
// 		delete panel;
// 		panel=NULL;

main_err1:
        delete spi;
        spi=NULL;
        sleep(2);

    std::cout<<"exit"<<std::endl;
    return 0;
}
