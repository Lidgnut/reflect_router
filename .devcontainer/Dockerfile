FROM vsavkov/clang-p2996:amd64

# Erweitere die Installationszeile im Dockerfile:
RUN apt-get update && apt-get install -y \
    sudo \
    git \
    cmake \
    ninja-build \
    gdb \
    && apt-get clean

# Falls GID/UID 1000 schon existiert: Löschen und neu anlegen als 'vscode'
RUN if getent passwd 1000; then userdel -f $(getent passwd 1000 | cut -d: -f1); fi \
    && if getent group 1000; then groupdel $(getent group 1000 | cut -d: -f1); fi \
    && groupadd --gid 1000 vscode \
    && useradd --uid 1000 --gid 1000 -m vscode \
    && echo vscode ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode

USER vscode


ENV LD_LIBRARY_PATH=/opt/p2996/clang/lib/x86_64-unknown-linux-gnu:$LD_LIBRARY_PATH

# Sicherstellen, dass clang++ gefunden wird
ENV CC=clang
ENV CXX=clang++