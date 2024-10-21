#include "common.h"

#define MAP_INFO_MAX_GRID_X 256
#define MAP_INFO_MAX_GRID_Y 256

typedef struct MapInfo 
{
    char Name[128];
    char Grid[MAP_INFO_MAX_GRID_Y][MAP_INFO_MAX_GRID_X];
    char MapWidth;
    char MapHeight;
} MapInfo_t;

bool Map_LoadMap(char* pFilePath, MapInfo_t* pInfoOut)
{
    memset(pInfoOut, 0, sizeof(*pInfoOut));

    FILE* pFile = fopen(pFilePath, "r");
    if (!pFile)
    {
        fprintf(stderr, "Failed to open map file %s\n", pFilePath);
        return false;
    }

    // Assuming a matrix of char values delimited by commas (a CSV, basically)
    int stride = -1;
    for (int y = 0; y < MAP_INFO_MAX_GRID_Y; ++y)
    {
        int x = 0;
        char* pLine = NULL;
        size_t lineLen = 0;
        if (getline(&pLine, &lineLen, pFile))
        {
            // Process the line and extract the comma-delimited values
            char* pToken = strtok(pLine, ",");
            while (pToken)
            {
                const char TokenAsInt = atoi(pToken);
                pInfoOut->Grid[y][x] = (char)TokenAsInt;
                ++x;

                pToken = strtok(NULL, ",");
            }

            free(pLine);
        }
        else
        {
            // EOF
            pInfoOut->MapHeight = y;
            break;
        }

        if (!pInfoOut->MapWidth)
        {
            pInfoOut->MapWidth = x;
        }
        else if (pInfoOut->MapWidth != x)
        {
            fprintf(stderr, "Inconsistent map width\n");
            return false;
        }

        pInfoOut->MapHeight = y;
    }

    return true;
}
