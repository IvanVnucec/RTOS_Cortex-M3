# run these two commands
# sudo docker build -t="put_name_here" .
# sudo docker run --rm put_name_here

FROM continuumio/miniconda3

RUN apt-get update -y
RUN apt-get install -y \
        qemu-system-arm 

COPY environment.yml tmp/environment.yml
COPY requirements.txt tmp/requirements.txt

RUN conda env create -f tmp/environment.yml

RUN echo "source activate env" > ~/.bashrc

ENV PATH /opt/conda/envs/env/bin:$PATH

COPY . .

CMD ["sh", "-c", "make"]