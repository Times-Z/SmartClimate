ARG DOCKER_TAG=release-v5.4
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV EDITOR=nvim
ARG USER=ubuntu

RUN set -xe; \
    apt-get update -y \
    && apt-get install udev neovim clang-tidy cppcheck -y \
    && echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> /home/${USER}/.bashrc \
    && git config --global --add safe.directory /workspaces

USER ${USER}

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]
