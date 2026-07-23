
#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class AudioController {
public:
    AudioController();
    ~AudioController();

    bool Initialize();
    void Shutdown();

    float GetMasterVolume();
    bool SetMasterVolume(float volume);
    bool IsMuted();
    bool SetMuted(bool muted);

private:
    IMMDeviceEnumerator* m_pEnumerator;
    IMMDevice* m_pDevice;
    IAudioEndpointVolume* m_pEndpointVolume;
};

#endif
