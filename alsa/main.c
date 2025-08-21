#include <stdio.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

// exit flag for signal handling
volatile sig_atomic_t exit_flag = 0;
// frames per period
static int frames = 4410;

// buffer structure
typedef struct
{
    char *buffer;
    int head_ptr;
    int tail_ptr;
    int size;
} buffer_t;

// handler for exit signals
void handle_exit(int sig)
{
    exit_flag = 1;
    printf("Exiting, pls waiting...\n");
}

/**
 * create a new PCM handle for playback or capture
 * @param handle pointer to the PCM handle
 * @param pcm_name name of the PCM device (e.g., "hw:1,0")
 * @param stream type of stream (SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE)
 * @return 0 on success, -1 on error
 */
int create_handle(snd_pcm_t **handle, const char *pcm_name, snd_pcm_stream_t stream)
{
    if (snd_pcm_open(handle, pcm_name, stream, 0) < 0)
    {
        perror("");
        return -1;
    }
    if (snd_pcm_set_params(*handle,
                           SND_PCM_FORMAT_S16_LE,
                           SND_PCM_ACCESS_RW_INTERLEAVED,
                           2,     // 通道数
                           44100, // 采样率
                           1,     // 是否允许软件调整
                           10000) // 延迟（微秒）
        < 0)
    {
        perror("");
        return -1;
    }
    return 0;
}

/**
 * Thread function for recording audio
 * @param arg pointer to the buffer structure
 * @return NULL
 */
void *record_thread(void *arg)
{
    buffer_t *buffer = (buffer_t *)arg;

    snd_pcm_t *pcm_capture_handle;
    create_handle(&pcm_capture_handle, "hw:1,0", SND_PCM_STREAM_CAPTURE);

    while (!exit_flag)
    {
        char *start_ptr = buffer->buffer + buffer->head_ptr;

        int real_rec_frames = 0;
        if ((real_rec_frames = snd_pcm_readi(pcm_capture_handle, start_ptr, frames)) < 0)
        {
            snd_pcm_recover(pcm_capture_handle, -EPIPE, 0);
            perror("");
        }

        buffer->head_ptr = buffer->head_ptr + real_rec_frames * 2 * 2;
    }

    snd_pcm_close(pcm_capture_handle);
    return NULL;
}

/**
 * Thread function for playing audio
 * @param arg pointer to the buffer structure
 * @return NULL
 */
void *play_thread(void *arg)
{
    buffer_t *buffer = (buffer_t *)arg;

    snd_pcm_t *pcm_play_handle;
    create_handle(&pcm_play_handle, "hw:1,0", SND_PCM_STREAM_PLAYBACK);

    while (!exit_flag)
    {
        int under_play = buffer->head_ptr - buffer->tail_ptr;
        char *start_ptr = buffer->buffer + buffer->tail_ptr;
        if (under_play == 0)
        {
            continue;
            printf("No data to play, waiting...\n");
        }
        if (under_play < 0)
            printf("wtf?");

        int real_play_frames = 0;
        if ((real_play_frames = snd_pcm_writei(pcm_play_handle, start_ptr, frames)) < 0)
        {
            snd_pcm_recover(pcm_play_handle, -EPIPE, 0);
            fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(errno));
        }

        buffer->tail_ptr = buffer->tail_ptr + real_play_frames * 2 * 2; // 2 bytes per sample * 2 channels
    }

    snd_pcm_close(pcm_play_handle);
    return NULL;
}

int main()
{
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);

    buffer_t buffer;
    buffer.size = 44100 * 2 * 2 * 500; //
    buffer.buffer = (char *)malloc(buffer.size);
    buffer.head_ptr = 0;
    buffer.tail_ptr = 0;

    pthread_t record_tid, play_tid;
    pthread_create(&record_tid, NULL, record_thread, &buffer);
    pthread_create(&play_tid, NULL, play_thread, &buffer);
    pthread_join(record_tid, NULL);
    pthread_join(play_tid, NULL);

    free(buffer.buffer);
    return 0;
}