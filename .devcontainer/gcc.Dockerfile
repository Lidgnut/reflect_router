FROM debian:unstable-slim

# Basis-Tools und GCC-Snapshot (Trunk) installieren
RUN apt-get update && apt-get install -y \
    gcc-snapshot \
    cmake \
    ninja-build \
    binutils \
    git \
    sudo \
    gdb \
    strace \
    && rm -rf /var/lib/apt/lists/*

# User 'vscode' anlegen, damit VS Code den Container starten kann
# Wir nutzen UID 1000, was der Standard für den ersten User unter Linux ist
RUN groupadd --gid 1000 vscode \
    && useradd --uid 1000 --gid 1000 -m vscode \
    && echo vscode ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode

# GCC Snapshot Pfade setzen
ENV PATH="/usr/lib/gcc-snapshot/bin:$PATH"
ENV LD_LIBRARY_PATH="/usr/lib/gcc-snapshot/lib:$LD_LIBRARY_PATH"

# Symlinks für g++ und gcc setzen, damit CMake sie sofort findet
RUN ln -sf /usr/lib/gcc-snapshot/bin/g++ /usr/bin/g++ && \
    ln -sf /usr/lib/gcc-snapshot/bin/gcc /usr/bin/gcc

WORKDIR /workspaces/cpp_reflection

# Als User 'vscode' arbeiten
USER vscode

# Kleiner Test beim Start
CMD ["g++", "--version"]