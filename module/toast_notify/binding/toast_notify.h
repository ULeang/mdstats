#ifndef TOAST_NOTIFY_H_
#define TOAST_NOTIFY_H_

#include <stdint.h>

extern "C" typedef enum Sound {
  /// The default system sound.
  Default,
  /// A sound typically used for instant messages.
  IM,
  /// A sound typically used for incoming mail.
  Mail,
  /// A sound typically used for reminders.
  Reminder,
  /// A sound typically used for incoming SMS messages.
  SMS,
  /// No sound.
  None,
} Sound;

extern "C" void toast_notify(
  const char *text1, const char *text2, const char *text3, Sound sound, uint64_t expire);

#endif
