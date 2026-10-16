#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define BUFFER_SIZE 65536

void process_packet(unsigned char* buffer, int size) {
    struct iphdr *iph = (struct iphdr*)buffer;
    unsigned short iphdrlen = iph->ihl * 4;
    
    struct sockaddr_in source, dest;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;

    if (iph->protocol == 6) {
        struct tcphdr *tcph = (struct tcphdr*)(buffer + iphdrlen);
        printf("[TCP] %s:%d -> ", inet_ntoa(source.sin_addr), ntohs(tcph->source));
        printf("%s:%d | Len: %d\n", inet_ntoa(dest.sin_addr), ntohs(tcph->dest), size);
    } else if (iph->protocol == 17) {
        struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen);
        printf("[UDP] %s:%d -> ", inet_ntoa(source.sin_addr), ntohs(udph->source));
        printf("%s:%d | Len: %d\n", inet_ntoa(dest.sin_addr), ntohs(udph->dest), size);
    }
}

int main() {
    int sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock_raw < 0) {
        perror("Error al crear el socket RAW");
        return 1;
    }

    unsigned char *buffer = (unsigned char*)malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Error de asignacion de memoria");
        close(sock_raw);
        return 1;
    }

    printf("Iniciando monitor de red en tiempo real...\n");

    for (int i = 0; i < 20; ++i) {
        int data_size = recvfrom(sock_raw, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (data_size < 0) {
            perror("Error al recibir el paquete");
            break;
        }
        process_packet(buffer, data_size);
    }

    free(buffer);
    close(sock_raw);
    return 0;
}
