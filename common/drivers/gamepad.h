#ifndef DRIVERS__GAMEPAD_H__
#define DRIVERS__GAMEPAD_H__

#if defined (UEFI)

#include <stdbool.h>
#include <efi.h>

bool gamepad_init(void);
void gamepad_deinit(void);
EFI_EVENT gamepad_get_efi_event(void);
int gamepad_get_key(void);

#endif

#endif
