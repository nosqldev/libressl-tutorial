package main

import (
    "crypto/rand"
    "crypto/tls"
    "net"
    "crypto/x509"
    "io/ioutil"
    "fmt"
    "io"
)

func process_connection(conn net.Conn) {
    defer conn.Close()
    buf := make([]byte, 512)
    for {
        fmt.Print("server: ready to read\n")
        n, err := conn.Read(buf)
        if err != nil {
            if err == io.EOF {
                break
            }
            if err != nil {
                fmt.Printf("conn.Read() failed: %s\n", err)
            }
            break
        }

        fmt.Printf("server: received %q\n", string(buf[:n]))

        n, err = conn.Write([]byte("world"))
        fmt.Println("server: wrote back to client")

        if err != nil {
            fmt.Printf("conn.Write() failed: %s\n", err)
            break
        }
    }
    fmt.Println("server: conn: closed")
}

func main() {
    ca_b, err := ioutil.ReadFile("../../autogen/key/cacert.der")
    if err != nil {
        fmt.Println("read cacert.der failed: ", err)
        return
    }
    ca, err := x509.ParseCertificate(ca_b)
    if err != nil {
        fmt.Println("parse cacert.der failed: ", err)
        return
    }

    priv_b, err := ioutil.ReadFile("../../autogen/key/cakey.der")
    if err != nil {
        fmt.Println("read cakey.der failed: ", err)
        return
    }

    priv, err := x509.ParsePKCS1PrivateKey(priv_b)
    if err != nil {
        fmt.Println("parse cakey.der failed: ", err)
        return
    }

    pool := x509.NewCertPool()
    pool.AddCert(ca)

    cert := tls.Certificate{
        Certificate: [][]byte{ ca_b },
        PrivateKey: priv,
    }

    config := tls.Config{
        ClientAuth: tls.NoClientCert,
        Certificates: []tls.Certificate{cert},
        ClientCAs: pool,
    }

    config.Rand = rand.Reader
    service := "0.0.0.0:9999"
    listener, err := tls.Listen("tcp", service, &config)
    if err != nil {
        fmt.Printf("tls.listen() failed: %s\n", err)
    }
    fmt.Println("server: listening")

    for {
        conn, err := listener.Accept()
        if err != nil {
            fmt.Printf("server: accept: %s\n", err)
            break
        }
        defer conn.Close()
        fmt.Printf("server: accepted from %s\n", conn.RemoteAddr())
        go process_connection(conn)
    }
}
