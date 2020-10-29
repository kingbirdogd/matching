FROM docker.coinflex-cn.com/core-awslinux2/coinflex-core-awslinux2:latest
MAINTAINER Peter
RUN yum install -y procps-ng less cronie logrotate psmisc telnet
RUN pip3 install simplejson requests
RUN echo "alias ll='ls -ltr'" >> /root/.bashrc
RUN mkdir -p /app/log
RUN mkdir -p /home/docker/usr/local
VOLUME /app/log
COPY bin     /home/docker/usr/local/
COPY include /home/docker/usr/local/
COPY lib     /home/docker/usr/local/
COPY lib64   /home/docker/usr/local/
COPY md.fbs  /app
COPY msg.fbs /app
#COPY zmq_proxy /app
COPY pulsar_proxy /app
#COPY xpubxsub /app
#COPY proxy_lws /app
#COPY proxy_md_lws /app
COPY proxy_md_pulsar /app
COPY test_md_tcp_server /app
COPY test_tcp_matching_server /app
COPY auction_tcp_matching_client /app
COPY start_core_v2_BCH.sh /app
COPY start_auction.sh /app
COPY run_auction.* /app/
COPY logrotate /app
COPY config_parser /app/config_parser
COPY start.sh /app
COPY me_server  /app
COPY me_server2 /app
#RUN cat /app/logrotate.cron >> /etc/crontab
WORKDIR /app
#ENTRYPOINT [ "sh", "-c", "/app/start_core_v2.sh", "aliyun-dev" ]
ENV PLSR_URL="PULSAR"
ENV REST_URL="RESTAPI"
ENV MARKET=""
ENTRYPOINT [ "sh", "-c", "/app/start.sh \$MARKET" ]
