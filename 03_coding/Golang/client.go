package main

import (
    "crypto/tls"
    "fmt"
    "io"
)

func main() {
    config := tls.Config {
        ClientAuth: tls.NoClientCert,
        InsecureSkipVerify: true,
    }
    conn, err := tls.Dial("tcp", "127.0.0.1:9999", &config)
    if err != nil {
        fmt.Printf("tls.Dial() failed: %s\n", err)
        return
    }
    defer conn.Close()

    state := conn.ConnectionState()
    fmt.Println("client connected to: ", conn.RemoteAddr())
    fmt.Println("client handshake: ", state.HandshakeComplete)
    fmt.Println("client mutual: ", state.NegotiatedProtocolIsMutual)

    message := "hello"
    n, err := io.WriteString(conn, message)
    if err != nil {
        fmt.Printf("io.WriteString() failed: %s\n", err)
        return
    }
    fmt.Printf("client wrote %q (%d bytes)\n", message, n)

    reply := make([]byte, 512)
    n, err = conn.Read(reply)
    fmt.Printf("client: read %q (%d bytes)\n", string(reply[:n]), n)
    fmt.Print("client: exiting\n")
}
