FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# 1. Install system dependencies (Including Doxygen and Graphviz for docs)
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    build-essential cmake git pkg-config \
    libpugixml-dev libjsoncpp-dev libarchive-dev libssl-dev \
    libboost-all-dev libtbb-dev qtbase5-dev qtchooser \
    qt5-qmake qtbase5-dev-tools libqt5svg5-dev \
    cbindgen catch2 doxygen graphviz libcurl4-openssl-dev curl

# 2. Install Rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# 3. Build libunrar
RUN git clone https://github.com/aawc/unrar.git /tmp/unrar \
    && cd /tmp/unrar \
    && make lib \
    && make install-lib \
    && cp dll.hpp /usr/local/include/ \
    && rm -rf /tmp/unrar

# 4. Build cpr v1.10.5
RUN git clone --depth 1 --branch 1.10.5 https://github.com/libcpr/cpr.git /tmp/cpr \
    && cd /tmp/cpr \
    && mkdir build && cd build \
    && cmake .. -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=ON \
    && cmake --build . --parallel $(nproc) \
    && cmake --install . \
    && rm -rf /tmp/cpr

# 5. Build libloot v0.25.0
RUN git clone --recursive --depth 1 --branch 0.25.0 https://github.com/loot/libloot.git /tmp/libloot \
    && cd /tmp/libloot \
    && mkdir build && cd build \
    && cmake .. -DLIBLOOT_STATIC=OFF -DLIBLOOT_INSTALL_DOCS=OFF \
    && cmake --build . --parallel $(nproc) \
    && cmake --install . \
    && rm -rf /tmp/libloot

ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH