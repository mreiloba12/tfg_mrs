#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if.h>

#define CAN_INTERFACE "can0"

// COB-IDs
#define COBID_JOYSTICK 0x186 //TPDO1 MANDO
#define COBID_CONTROLADORA_DIR 0x205 //RPDO1 CONTROLADORA

// Función para abrir el socket CAN
int open_can_socket() {
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Error al abrir el socket CAN");
        return -1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, CAN_INTERFACE, sizeof(ifr.ifr_name) - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("Error al obtener el índice de la interfaz CAN");
        close(s);
        return -1;
    }

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error al enlazar el socket CAN");
        close(s);
        return -1;
    }
    return s;
}

// Función para mapear el joystick
long map_joystick_to_position(unsigned char joystick_value) {
    long position=0x00000000;

    if (joystick_value >= 0x80 && joystick_value <= 0xFE) {
        // Mapeo de [0x80, 0xFE] a [0x0001, 0x03E8]
        position = (long)(((joystick_value - 0x80) * (0x03E8 - 0x0001)) / (0xFE - 0x80) + 1);
    }
    if (joystick_value >= 0x00 && joystick_value < 0x7F) {
        // Mapeo de [0x00, 0x7E] a [0xFFFFFC18, 0xFFFFFFFF]
        //position = (long)(0xFFFFFC18 - (joystick_value * 0xFFFFFC18 / 0x7E));
        position = (long)(0xFFFFFC18 + ((joystick_value * (0xFFFFFFFF - 0xFFFFFC18)) / 0x7E));
    }
    return position;
}

// Función para enviar mensajes CAN
void send_can_message(int s, unsigned int cobid, long position) {

    struct can_frame frame;
    frame.can_id = cobid;
    frame.can_dlc = 4;
    frame.data[0] = position;
    frame.data[1] = position >> 8;
    frame.data[2] = position >> 16;
    frame.data[3] = position >> 24;

    if (write(s, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("Error al enviar mensaje CAN");
    } else {
        printf("Mensaje CAN enviado a 0x%X: [%X %X %X %X]\n", cobid, frame.data[0], frame.data[1], frame.data[2], frame.data[3]);
    }
}

int main() {
    int s = open_can_socket();
    if (s < 0) return 1;
	open_can_socket();
    struct can_frame frame;

    while (1) {
        int nbytes = read(s, &frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("Error al leer del bus CAN");
            return 1;
        }

        if (frame.can_id == COBID_JOYSTICK) {
            unsigned char joystick_value = frame.data[3];
            long position = map_joystick_to_position (joystick_value);
            printf("Valor de posicion en hexadecimal: 0x%08lX\n", position);

            send_can_message(s, COBID_CONTROLADORA_DIR, position);
        }
    }

    close(s);
    return 0;
}
