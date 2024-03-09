SHELL = /bin/bash

PREFIX_PATH := ${HOME}/.local
TEMP_PATH := /tmp/heartbeat
CWD_PATH = $(shell pwd)

target/client: build_client
target/server: build_server

deps: deps_structure deps_grpc deps_server

deps_structure:
	mkdir -p ${PREFIX_PATH}
	mkdir -p ${TEMP_PATH}

deps_grpc:
	if ! command -v protoc &> /dev/null; then \
		if [ ! -d "${TEMP_PATH}/grpc" ]; then \
			git clone \
				--recurse-submodules \
				-b v1.62.0 \
				--depth 1 \
				--shallow-submodules https://github.com/grpc/grpc \
				${TEMP_PATH}/grpc; \
		fi; \
		cd ${TEMP_PATH}/grpc/ && \
			mkdir -p ${TEMP_PATH}/grpc/cmake/build/ && \
			pushd ${TEMP_PATH}/grpc/cmake/build/ && \
			cmake -DgRPC_INSTALL=ON \
			-DgRPC_BUILD_TESTS=OFF \
			-DCMAKE_INSTALL_PREFIX=${PREFIX_PATH} \
			../.. && \
			make -j $(nproc) && \
			make install; \
	fi

deps_server:
	go install google.golang.org/protobuf/cmd/protoc-gen-go@v1.28
	go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.2

build: build_server build_client

build_deps: deps

build_client: protoc_client
	mkdir -p ${CWD_PATH}/target
	mkdir -p ${TEMP_PATH}/client
	cd ${CWD_PATH}/client && \
		cmake -B ${TEMP_PATH}/client -S . && \
		cmake --build ${TEMP_PATH}/client
	mv ${TEMP_PATH}/client/client ${CWD_PATH}/target/

build_server: protoc_server
	cd ${CWD_PATH}/server && \
		go build -o ${CWD_PATH}/target/server

protoc_client:
	protoc \
    		--proto_path=${CWD_PATH}/proto \
    		--cpp_out=${CWD_PATH}/client \
    		--grpc_out=${CWD_PATH}/client \
    		--plugin=protoc-gen-grpc=$(shell which grpc_cpp_plugin) \
    		heartbeat.proto

protoc_server:
	protoc \
		--proto_path=${CWD_PATH}/proto \
		--go_out=${CWD_PATH}/server/ \
		--go-grpc_out=${CWD_PATH}/server/ \
		--plugin=protoc-gen-go=$(shell go env GOPATH)/bin/protoc-gen-go \
		--plugin=protoc-gen-go-grpc=$(shell go env GOPATH)/bin/protoc-gen-go-grpc \
		heartbeat.proto

install: install_server install_client

install_client: target/client
	cp target/client ${PREFIX_PATH}/bin/heartbeat_client

install_server: target/server
	cp target/server ${PREFIX_PATH}/bin/heartbeat_server

uninstall: uninstall_server uninstall_client

uninstall_client:
	if [ -f "${PREFIX_PATH}/bin/heartbeat_client" ]; then \
		rm -rf ${PREFIX_PATH}/bin/heartbeat_client; \
	fi

uninstall_server:
	if [ -f "${PREFIX_PATH}/bin/heartbeat_server" ]; then \
		rm -rf ${PREFIX_PATH}/bin/heartbeat_server; \
	fi

clean:
	rm -rf ${CWD_PATH}/target
	rm -rf ${TEMP_PATH}