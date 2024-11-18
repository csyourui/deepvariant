# Copyright 2019 Google LLC.
# This is used to build the DeepVariant release docker image.
# It can also be used to build local images, especially if you've made changes
# to the code.
# Example command:
# $ git clone https://github.com/google/deepvariant.git
# $ cd deepvariant
# $ sudo docker build -t deepvariant .
#
# To build for GPU, use a command like:
# $ sudo docker build --build-arg=FROM_IMAGE=nvidia/cuda:11.3.1-cudnn8-devel-ubuntu20.04 --build-arg=DV_GPU_BUILD=1 -t deepvariant_gpu .


ARG FROM_IMAGE=nvidia/cuda:11.3.1-cudnn8-devel-ubuntu20.04
# PYTHON_VERSION is also set in settings.sh.
ARG PYTHON_VERSION=3.8
ARG DV_GPU_BUILD=0
ARG VERSION=1.6.0

FROM continuumio/miniconda3 as conda_setup
RUN conda config --add channels defaults && \
    conda config --add channels bioconda && \
    conda config --add channels conda-forge
RUN conda create -n bio \
                    bioconda::bcftools=1.10 \
                    bioconda::samtools=1.10 \
    && conda clean -a

FROM ${FROM_IMAGE}
COPY --from=conda_setup /opt/conda /opt/conda
LABEL maintainer="https://github.com/google/deepvariant/issues"

ARG DV_GPU_BUILD
ENV DV_GPU_BUILD=${DV_GPU_BUILD}

# Copying DeepVariant source code
COPY . /opt/deepvariant

ARG VERSION
ENV VERSION=${VERSION}

WORKDIR /opt/deepvariant

RUN echo "Acquire::http::proxy \"$http_proxy\";\n" \
         "Acquire::https::proxy \"$https_proxy\";" > "/etc/apt/apt.conf"

RUN ./build-prereq.sh \
  && PATH="${HOME}/bin:${PATH}" ./build_release_binaries.sh  # PATH for bazel
