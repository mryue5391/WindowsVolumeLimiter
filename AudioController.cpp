
#include "AudioController.h"

AudioController::AudioController()
    : m_pEnumerator(nullptr)
    , m_pDevice(nullptr)
    , m_pEndpointVolume(nullptr)
{
}

AudioController::~AudioController()
{
    Shutdown();
}

bool AudioController::Initialize()
{
    HRESULT hr;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_pEnumerator
    );

    if (FAILED(hr)) {
        return false;
    }

    hr = m_pEnumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &m_pDevice
    );

    if (FAILED(hr)) {
        Shutdown();
        return false;
    }

    hr = m_pDevice->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        nullptr,
        (void**)&m_pEndpointVolume
    );

    if (FAILED(hr)) {
        Shutdown();
        return false;
    }

    return true;
}

void AudioController::Shutdown()
{
    if (m_pEndpointVolume) {
        m_pEndpointVolume->Release();
        m_pEndpointVolume = nullptr;
    }
    if (m_pDevice) {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
    if (m_pEnumerator) {
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
    }
}

float AudioController::GetMasterVolume()
{
    if (!m_pEndpointVolume) {
        return 0.0f;
    }

    float volume = 0.0f;
    m_pEndpointVolume->GetMasterVolumeLevelScalar(&volume);
    return volume;
}

bool AudioController::SetMasterVolume(float volume)
{
    if (!m_pEndpointVolume) {
        return false;
    }

    HRESULT hr = m_pEndpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
    return SUCCEEDED(hr);
}

bool AudioController::IsMuted()
{
    if (!m_pEndpointVolume) {
        return false;
    }

    BOOL muted = FALSE;
    m_pEndpointVolume->GetMute(&muted);
    return muted != FALSE;
}

bool AudioController::SetMuted(bool muted)
{
    if (!m_pEndpointVolume) {
        return false;
    }

    HRESULT hr = m_pEndpointVolume->SetMute(muted ? TRUE : FALSE, nullptr);
    return SUCCEEDED(hr);
}
