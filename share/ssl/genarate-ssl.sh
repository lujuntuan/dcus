#!/bin/sh
#Need [openssl] and [keytool]
CURRENT_DIR="$(cd "$(dirname "$BASH_SOURCE")"&&pwd)"

usage() {
    echo -e "Usage:\n\
        genarate-ssl.sh local\n\
        genarate-ssl.sh web\n\
    "
}

prepare() {
    echo -e "********** Prepare...\n"
    [ -d ${SSL_DIR} ] && rm -rf ${SSL_DIR}
    mkdir -p ${SSL_DIR}
}

genarate() {
    NAME=${SSL_TYPE}-${1}
    CONFIG=${CURRENT_DIR}/ssl-${1}.cnf
    echo -e "********** Genarate ${NAME} key...\n"
    openssl genrsa \
        -out ${SSL_DIR}/${NAME}.key \
        2048
    echo -e "********** Genarate ${NAME} csr...\n"
    openssl req \
        -new \
        -batch \
        -config ${CONFIG} \
        -key ${SSL_DIR}/${NAME}.key \
        -out ${SSL_DIR}/${NAME}.csr
    echo -e "********** Genarate ${NAME} crt...\n"
    if [ "${NAME}" = "${SSL_TYPE}-ca" ]; then
        openssl x509 \
            -req \
            -extensions v3_ca_ext \
            -extfile ${CONFIG} \
            -days ${SSL_DAYS} \
            -in ${SSL_DIR}/${NAME}.csr \
            -out ${SSL_DIR}/${NAME}.crt \
            -signkey ${SSL_DIR}/${NAME}.key
    else
        openssl x509 \
            -req \
            -extensions v3_req_ext \
            -extfile ${CONFIG} \
            -days ${SSL_DAYS} \
            -in ${SSL_DIR}/${NAME}.csr \
            -out ${SSL_DIR}/${NAME}.crt \
            -CAcreateserial \
            -CA ${SSL_DIR}/${SSL_TYPE}-ca.crt \
            -CAkey ${SSL_DIR}/${SSL_TYPE}-ca.key
        echo -e "********** Genarate ${NAME} p12...\n"
        openssl pkcs12 \
            -export \
            -clcerts \
            -passout pass:${SSL_PASSWORD} \
            -inkey ${SSL_DIR}/${NAME}.key \
            -in ${SSL_DIR}/${NAME}.crt \
            -out ${SSL_DIR}/${NAME}.p12
        echo -e "********** Genarate ${NAME} jks...\n"
        keytool -importkeystore \
            -noprompt \
            -srcstoretype pkcs12 \
            -deststoretype pkcs12 \
            -alias 1 \
            -srcstorepass ${SSL_PASSWORD} \
            -deststorepass ${SSL_PASSWORD} \
            -srckeystore ${SSL_DIR}/${NAME}.p12 \
            -destkeystore ${SSL_DIR}/${NAME}.jks
    fi
}

###################################################################

if [ $# -ne 1 ]; then
    echo -e "********** Wrong number of parameters\n"
    usage
    exit 1
fi

SSL_TYPE=$1
SSL_DIR=${CURRENT_DIR}/${SSL_TYPE}
SSL_PASSWORD=123456789
SSL_DAYS=36500

if [ "${SSL_TYPE}" = "local" ]; then
    echo -e "********** Type: local\n"
    prepare
    genarate ca
    genarate server
elif [ "${SSL_TYPE}" = "web" ]; then
    echo -e "********** Type: web\n"
    prepare
    genarate ca
    genarate server
else
    echo -e "********** Parameter input error\n"
    usage
    exit 2
fi

echo -e "********** Done.\n"
exit 0
