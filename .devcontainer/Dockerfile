ARG DOCKER_TAG=release-v5.4
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV EDITOR=nvim
ARG USER=ubuntu

RUN set -xe; \
    apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y \
        build-essential \
        clang-tidy \
        cppcheck \
        neovim \
        udev \
        usbutils \
        sudo \
    && echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> /home/${USER}/.bashrc \
    && chown -R ${USER}:${USER} /usr \
    && usermod -aG sudo ${USER} \
    && chown root:root /usr/bin/sudo \
    && chmod 4755 /usr/bin/sudo \
    && echo "${USER} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/${USER} \
    && git config --global --add safe.directory /

USER ${USER}

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]
