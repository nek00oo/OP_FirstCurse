#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

FILE *InputFile;
FILE *OutputFile;

#pragma pack(push, 1)
typedef struct {
    char ID3[3];
    unsigned char version[2];
    unsigned char flags;
    unsigned char size[4];
} MP3Header;

typedef struct {
    char ID[4];
    unsigned char size[4];
    char flags[2];
} FrameHeader;

#pragma pack(pop)

unsigned int SizeInt(const unsigned char *size) {
    unsigned int NewSize = 0;
    for (int i = 0; i < 4; ++i) {
        NewSize += (unsigned int) ((size[i]) * pow(2, 7 * (3 - i)));
    }
    return NewSize;
}

void SizeInMP3File(unsigned int value, char *args) {
    for (int i = 0; i < 4; i++) {
        args[3 - i] = value & 0x7f;
        value >>= 7;
    }
}


unsigned int ShowHeaderMP3ID3V2(MP3Header *MP, bool show_flag) {
    fseek(InputFile, 0, SEEK_SET);
    unsigned int SizeIntHeader;
    if (show_flag) {
        fseek(InputFile, 0, SEEK_SET);

        fread(MP, sizeof(MP3Header), 1, InputFile);
        printf("------------------------------------\n");
        printf("Header: %c%c%c\n", MP->ID3[0], MP->ID3[1], MP->ID3[2]);
        printf("Version: %d.%d\n", MP->version[0], MP->version[1]);

        printf("Flag Unsynchronisation: %d\n", (MP->flags >> 7) & 1);
        printf("Flag Extended header: %d\n", (MP->flags >> 6) & 1);
        printf("Flag Experimental indicator: %d\n", (MP->flags >> 5) & 1);

        SizeIntHeader = SizeInt(MP->size);
        printf("Size HeaderMp3: %u\n", SizeIntHeader);
        printf("------------------------------------\n");

    }
    if (!show_flag) {
        fseek(InputFile, 0, SEEK_SET);
        fread(MP, sizeof(MP3Header), 1, InputFile);
        SizeIntHeader = SizeInt(MP->size);
    }
    if ((MP->flags >> 6) & 1) {
        int ext_header_size;
        fread(&ext_header_size, 1, 1, InputFile);
        char ext_header[ext_header_size];
        fread(ext_header, 1, ext_header_size, InputFile);
    }
    return SizeIntHeader;

}

void ShowHeaderFrame() {
    FrameHeader *FR = (FrameHeader *) malloc(sizeof(FrameHeader));
    for (int frame = 0; fread(FR->ID, 1, 4, InputFile) > 0 && FR->ID[0] != 0; frame++) {
        printf("__________________________________________________________\n");
        printf("Tag: %.4s\n", FR->ID);
        fread(FR->size, 1, 4, InputFile);

        unsigned int size_int = SizeInt(FR->size);
        printf("SizeFrame: %u\n", size_int);

        fread(FR->flags, 1, 2, InputFile);
        printf("Flags: %d %d\n", FR->flags[0], FR->flags[1]);

        if (FR->ID[0] == 'T') {
            char EncodingTextFrame;
            fread(&EncodingTextFrame, 1, 1, InputFile);
            char InformationText[size_int - 1];
            fread(InformationText, 1, size_int - 1, InputFile);
            printf("Data: ");
            for (; size_int > 1 && InformationText[size_int - 2] == 0; size_int--);
            for (int i = 0; i < size_int - 1; i++) {
                if (InformationText[i] == 0)
                    printf(" | ");
                else
                    printf("%c", InformationText[i]);
            }
            printf("\n");
        } else {
            char data[size_int];
            fread(data, 1, size_int, InputFile);
        }

    }
    printf("__________________________________________________________\n");
    free(FR);
}

void ReadSingleFrame(const char *TagFrame) {
    bool find_tag_flag = false;
    FrameHeader *FR = (FrameHeader *) malloc(sizeof(FrameHeader));
    printf("///////////////////////////\n");
    for (int frame = 0; fread(FR->ID, 1, 4, InputFile) > 0 && FR->ID[0] != 0; frame++) {

        bool show_frame_flag = (FR->ID[0] == TagFrame[0] && FR->ID[1] == TagFrame[1] && FR->ID[2] == TagFrame[2] &&
                                FR->ID[3] == TagFrame[3]);

        if (show_frame_flag){
            printf("Tag: %.4s\n", FR->ID);
            find_tag_flag = true;
        }

        fread(FR->size, 1, 4, InputFile);
        unsigned int size_int = SizeInt(FR->size);
        if (show_frame_flag) printf("Size: %d\n", size_int);

        fread(&FR->flags, 1, 2, InputFile);
        if (show_frame_flag) printf("Flags: %d %d\n", FR->flags[0], FR->flags[1]);

        if (FR->ID[0] == 'T') {
            char EncodingTextFrame;
            fread(&EncodingTextFrame, 1, 1, InputFile);
            char data[size_int - 1];
            fread(data, 1, size_int - 1, InputFile);
            if (show_frame_flag) {
                printf("Data: ");
                for (; size_int > 1 && data[size_int - 2] == 0; size_int--);
                for (int i = 0; i < size_int - 1; i++) {
                    if (data[i] == 0)
                        printf(" | ");
                    else
                        printf("%c", data[i]);
                }
                printf("\n");
            }
        } else {
            char data[size_int];
            fread(data, 1, size_int, InputFile);
        }
    }
    if (!find_tag_flag){
        printf("Tag not find\n");
    }
    printf("///////////////////////////\n");
    free(FR);
}

void SetFrame(MP3Header *MP, char *Frame, char *prop_value, unsigned int size) {

    fseek(InputFile, 0, SEEK_SET);
    fread(MP, sizeof(MP3Header), 1, InputFile);

    fwrite(MP->ID3, 1, 3, OutputFile);
    fwrite(MP->version, 1, 2, OutputFile);
    fwrite(&MP->flags, 1, 1, OutputFile);
    fwrite(MP->size, 1, 4, OutputFile);

    if ((MP->flags >> 6) & 1) {
        int ExtendHeaderSize;
        fread(&ExtendHeaderSize, 1, 1, InputFile);
        char ExtendHeader[ExtendHeaderSize];
        fread(ExtendHeader, 1, ExtendHeaderSize, InputFile);

        fwrite(&ExtendHeaderSize, 1, 1, OutputFile);
        fwrite(ExtendHeader, 1, ExtendHeaderSize, OutputFile);

    }

    unsigned int count = strlen(prop_value) + 1;
    size += count + 10;

    char new_size[4] = {0};
    char encoding = 03;
    char new_flags[2] = {0};
    SizeInMP3File(count, new_size);
    fwrite(Frame, 1, 4, OutputFile);
    fwrite(new_size, 1, 4, OutputFile);
    fwrite(new_flags, 1, 2, OutputFile);
    fwrite(&encoding, 1, 1, OutputFile);
    fwrite(prop_value, 1, count - 1, OutputFile);

    FrameHeader *FM = (FrameHeader *) malloc(sizeof(FrameHeader));

    for (int frame = 0; fread(FM->ID, 1, 4, InputFile) > 0 && FM->ID[0] != 0; frame++) {
        bool set = FM->ID[0] == Frame[0] && FM->ID[1] == Frame[1] && FM->ID[2] == Frame[2] && FM->ID[3] == Frame[3];

        fread(FM->size, 1, 4, InputFile);
        fread(FM->flags, 1, 2, InputFile);
        unsigned int size_int = SizeInt(FM->size);
        char EncodingTextFrame;
        char data[size_int];
        fread(&EncodingTextFrame, 1, 1, InputFile);
        fread(data, 1, size_int - 1, InputFile);

        if (!set) {
            fwrite(FM->ID, 1, 4, OutputFile);
            fwrite(FM->size, 1, 4, OutputFile);
            fwrite(FM->flags, 1, 2, OutputFile);
            fwrite(&EncodingTextFrame, 1, 1, OutputFile);
            fwrite(data, 1, size_int - 1, OutputFile);
        } else {
            size -= size_int;
        }
    }
    while (!feof(InputFile)) {
        char LastInformationInFile[1500];
        unsigned int cnt = fread(&LastInformationInFile, 1, 1500, InputFile);
        fwrite(&LastInformationInFile, 1, cnt, OutputFile);
    }

    char NewHeaderSize[4];
    SizeInMP3File(size, NewHeaderSize);
    fseek(OutputFile, 6, SEEK_SET);
    fwrite(NewHeaderSize, 1, 4, OutputFile);
}


int main(int argc, char *argv[]) {

    const int command_set = 0, command_get = 1, command_show = 2;
    int cmd = -1;
    char *file_path = NULL;
    char *prop_value = NULL;
    char *frame_tag = NULL;

    MP3Header *MP = (MP3Header *) malloc((sizeof(MP3Header)));

    for (int i = 0; i < argc; i++) {
        if (!strncmp(argv[i], "--filepath=", 11)) {
            file_path = argv[i] + 11;
        }
    }
    for (int i = 0; i < argc; i++) {
        if (!strncmp(argv[i], "--value=", 8)) {
            prop_value = argv[i] + 8;
        }
    }
    for (int i = 0; i < argc; i++) {
        int new_cmd;
        if (!strncmp(argv[i], "--show", 6)) {
            new_cmd = command_show;
        } else if (!strncmp(argv[i], "--set=", 6)) {
            new_cmd = command_set;
            frame_tag = argv[i] + 6;
        } else if (!strncmp(argv[i], "--get=", 6)) {
            new_cmd = command_get;
            frame_tag = argv[i] + 6;
        } else {
            continue;
        }
        if (cmd != -1) {
            printf("Only one command is supported at a time\n");
            return 0;
        }
        cmd = new_cmd;
    }
    if (cmd == -1) {
        printf("Command not found\n");
        return 0;
    }
    if (file_path == NULL) {
        printf("File not found\n");
        return 0;
    }
    if (argc > 4 || cmd != command_set && argc > 3) {
        printf("Many argument.You can call the command --show or --get=\"prop_name\" or --set=\"prop_name\" --value=\"prop_value\"\n");
        return 0;
    }
    InputFile = fopen(file_path, "rb");
    if (cmd == command_show) {
        ShowHeaderMP3ID3V2(MP, 1);
        ShowHeaderFrame();
    } else if (cmd == command_get) {
        if (frame_tag != NULL) {
            ShowHeaderMP3ID3V2(MP, 0);
            ReadSingleFrame(frame_tag);
        } else {
            printf("Tag Frame not found");
        }
    } else if (cmd == command_set) {
        if (frame_tag != NULL && prop_value != NULL) {
            OutputFile = fopen("out.mp3", "wb");
            unsigned int size = ShowHeaderMP3ID3V2(MP, 0);
            SetFrame(MP, frame_tag, prop_value, size);
            fclose(OutputFile);
        } else if (frame_tag == NULL) {
            printf("Tag Frame not found");
        } else {
            printf("Value not found");
        }
    }
    free(MP);
    fclose(InputFile);
    return 0;
}

