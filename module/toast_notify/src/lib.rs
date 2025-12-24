use std::ffi::{CStr, c_char};
use std::time::Duration;
use winrt_toast_reborn::{Toast, ToastManager};

#[repr(C)]
pub enum Sound {
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
}
impl Sound {
    fn to_toast_sound(self) -> winrt_toast_reborn::content::audio::Sound {
        use winrt_toast_reborn::content::audio::Sound as TSound;
        match self {
            Sound::Default => TSound::Default,
            Sound::IM => TSound::IM,
            Sound::Mail => TSound::Mail,
            Sound::Reminder => TSound::Reminder,
            Sound::SMS => TSound::SMS,
            Sound::None => TSound::None,
        }
    }
}
impl Into<winrt_toast_reborn::content::audio::Sound> for Sound {
    fn into(self) -> winrt_toast_reborn::content::audio::Sound {
        self.to_toast_sound()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn toast_notify(
    text1: *const c_char,
    text2: *const c_char,
    text3: *const c_char,
    sound: Sound,
    expire: u64,
) {
    let manager = ToastManager::new(ToastManager::POWERSHELL_AUM_ID);

    let mut toast = Toast::new();
    toast
        .text1(unsafe { CStr::from_ptr(text1) }.to_str().unwrap())
        .text2(unsafe { CStr::from_ptr(text2) }.to_str().unwrap())
        .text3(unsafe { CStr::from_ptr(text3) }.to_str().unwrap())
        .audio(winrt_toast_reborn::Audio::new(sound.into()))
        .expires_in(Duration::from_secs(expire));

    let _ = manager.show(&toast);
}
