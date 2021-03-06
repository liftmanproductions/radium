#ifndef _RADIUM_AUDIO_SAMPLERECORDER_PROC_H
#define _RADIUM_AUDIO_SAMPLERECORDER_PROC_H

extern LANGSPEC void SampleRecorder_called_regularly(void);
extern LANGSPEC void RT_SampleRecorder_start_recording(struct Patch *patch, wchar_t *pathdir, int num_channels, float middle_note);
extern LANGSPEC void RT_SampleRecorder_stop_recording(struct Patch *patch);
extern LANGSPEC void RT_SampleRecorder_add_audio(struct Patch *patch, float **audio, int num_frames, int num_channels);
extern LANGSPEC void SampleRecorder_Init(void);

#endif

