# run these two commands
# sudo docker build -t="put_name_here" .
# sudo docker run --rm put_name_here

FROM ubuntu:20.04

ENV REFRESHED_AT 2017-12-01

WORKDIR /home/dev

RUN apt-get update -y
RUN apt-get install -y \
        qemu-system-arm \
        wget \
        make

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/6-2017q2/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 \
    && tar xvf gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 \
    && rm gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2

# Set up the compiler path
ENV PATH="/home/dev/gcc-arm-none-eabi-6-2017-q2-update/bin:${PATH}"

WORKDIR /usr/project

COPY . .

CMD ["sh", "-c", "make"]