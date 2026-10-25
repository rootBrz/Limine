#if defined (UEFI)

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <drivers/gamepad.h>
#include <lib/config.h>
#include <lib/getchar.h>
#include <lib/libc.h>
#include <lib/misc.h>
#include <efi.h>

// USB interface class of the wired Xbox 360 controller protocol.
#define XINPUT_CLASS 0xff
#define XINPUT_SUBCLASS 0x5d
#define XINPUT_PROTOCOL 0x01

#define QUEUE_SIZE 16

static const struct {
    uint16_t mask;
    int key;
} key_map[] = {
    { 1 << 0, GETCHAR_CURSOR_UP },
    { 1 << 1, GETCHAR_CURSOR_DOWN },
    { 1 << 2, GETCHAR_CURSOR_LEFT },
    { 1 << 3, GETCHAR_CURSOR_RIGHT },
    { 1 << 4, '\n' }, // Start
    { 1 << 8, GETCHAR_PGUP }, // LB
    { 1 << 9, GETCHAR_PGDOWN }, // RB
    { 1 << 12, '\n' }, // A
    { 1 << 13, GETCHAR_ESCAPE }, // B
    { 1 << 14, 'e' }, // X
};

static EFI_USB_IO_PROTOCOL *pad;
static uint8_t pad_endpoint;
static EFI_EVENT key_event;
static uint16_t prev_buttons;
static volatile size_t queue_head, queue_tail;
static int queue[QUEUE_SIZE];

// Runs at TPL_NOTIFY, so it only decodes and queues.
// shortcut: no auto-repeat for held buttons, add a timer if holding D-pad matters.
static EFI_STATUS EFI_API report_callback(
    void *data, EFI_UINTN length, void *context, uint32_t status) {
    (void)context;

    // XInput input report: type 0x00, size 0x14, then the 16 button bits.
    const uint8_t *r = data;
    if (status != 0 || r == NULL || length < 4 || r[0] != 0x00 || r[1] != 0x14) {
        return EFI_SUCCESS;
    }

    uint16_t buttons = r[2] | (uint16_t)r[3] << 8;
    uint16_t pressed = buttons & ~prev_buttons;
    prev_buttons = buttons;

    bool queued = false;
    for (size_t i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        size_t next = (queue_head + 1) % QUEUE_SIZE;
        if ((pressed & key_map[i].mask) != 0 && next != queue_tail) {
            queue[queue_head] = key_map[i].key;
            queue_head = next;
            queued = true;
        }
    }

    if (queued) {
        gBS->SignalEvent(key_event);
    }
    return EFI_SUCCESS;
}

static bool find_pad(void) {
    EFI_GUID guid = EFI_USB_IO_PROTOCOL_GUID;
    EFI_UINTN count = 0;
    EFI_HANDLE *handles = NULL;

    if (gBS->LocateHandleBuffer(EfiByProtocol, &guid, NULL, &count, &handles) != EFI_SUCCESS) {
        return false;
    }

    // shortcut: first controller only, add a per-pad state if more are needed.
    for (EFI_UINTN i = 0; i < count && pad == NULL; i++) {
        EFI_USB_IO_PROTOCOL *io;
        EFI_USB_INTERFACE_DESCRIPTOR id;

        if (gBS->HandleProtocol(handles[i], &guid, (void **)&io) != EFI_SUCCESS
         || io->UsbGetInterfaceDescriptor(io, &id) != EFI_SUCCESS
         || id.InterfaceClass != XINPUT_CLASS
         || id.InterfaceSubClass != XINPUT_SUBCLASS
         || id.InterfaceProtocol != XINPUT_PROTOCOL) {
            continue;
        }

        for (uint8_t e = 0; e < id.NumEndpoints; e++) {
            EFI_USB_ENDPOINT_DESCRIPTOR ed;
            if (io->UsbGetEndpointDescriptor(io, e, &ed) != EFI_SUCCESS) {
                continue;
            }
            // Interrupt IN.
            if ((ed.EndpointAddress & 0x80) != 0 && (ed.Attributes & 3) == 3) {
                pad = io;
                pad_endpoint = ed.EndpointAddress;
                break;
            }
        }
    }

    gBS->FreePool(handles);
    return pad != NULL;
}

bool gamepad_init(void) {
    gamepad_deinit();

    char *conf = config_get_value(NULL, 0, "GAMEPAD");
    if ((conf != NULL && strcmp(conf, "no") == 0) || efi_boot_services_exited) {
        return false;
    }

    if (!find_pad()) {
        return false;
    }

    prev_buttons = 0;
    queue_head = queue_tail = 0;

    if (gBS->CreateEvent(0, 0, NULL, NULL, &key_event) != EFI_SUCCESS
     || pad->UsbAsyncInterruptTransfer(pad, pad_endpoint, true, 8, 32,
            report_callback, NULL) != EFI_SUCCESS) {
        gamepad_deinit();
        return false;
    }

    return true;
}

void gamepad_deinit(void) {
    if (pad != NULL && key_event != NULL && !efi_boot_services_exited) {
        pad->UsbAsyncInterruptTransfer(pad, pad_endpoint, false, 0, 0, NULL, NULL);
    }
    if (key_event != NULL && !efi_boot_services_exited) {
        gBS->CloseEvent(key_event);
    }
    pad = NULL;
    key_event = NULL;
}

EFI_EVENT gamepad_get_efi_event(void) {
    return key_event;
}

int gamepad_get_key(void) {
    if (queue_tail == queue_head) {
        return 0;
    }
    int key = queue[queue_tail];
    queue_tail = (queue_tail + 1) % QUEUE_SIZE;
    return key;
}

#endif
