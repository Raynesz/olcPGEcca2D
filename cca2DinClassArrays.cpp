/*
IF USING STACK MEMORY FOR THE APP THEN INCREASE STACK RESERVE SIZE TO ABOUT 3 MB
MSVC: Project > Properties > Linker > Configuration Properties > Linker > System > Stack Reserve Size: 3145728

By default, the main application will use the heap memory. That can be changed in the main function.
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <stdlib.h>

// MODIFY THESE CONSTANTS TO CHANGE THE ACTIVE RULE ///////
#define RANGE 6
#define THRESHOLD 7
#define COLORS 18
#define NH false    // true for Moore, false for vonNeumann
///////////////////////////////////////////////////////////

#define TOT_SIZE 580
#define INNER_SIZE 540
#define MARGIN 15
#define BORDER 5

struct Color {
	int red;
	int green;
	int blue;
};

class CCA : public olc::PixelGameEngine
{
private:
	int range = RANGE;
	int threshold = THRESHOLD;
	int colorNumber = COLORS;
	bool moore = NH;
	std::vector<Color> colorPalette = {};
    int successorIndex;
	static const int margin = MARGIN;
	static const int border = BORDER;
	static const int textureWidth = INNER_SIZE + (2 * margin) + (2 * border);
	static const int textureHeight = INNER_SIZE + (2 * margin) + (2 * border);
	static const int xStartIndex = margin + border; // starting and ending iteration after and before the borders
	static const int xEndIndex = textureWidth - margin - border;
	static const int yStartIndex = margin + border;
	static const int yEndIndex = textureHeight - margin - border;
    //using 2 alternating arrays, 1 to read from and 1 to write to
    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> m_first = {};
    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> m_second = {};
    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> *curr = &m_first;
    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> *next = &m_second;
    bool alter = true;

public:
	CCA()
	{
		sAppName = "CCA2D";
	}

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
        std::cout << "Active Rule: R" << range << ", T" << threshold << ", C" << colorNumber;
        if (moore) std::cout << ", Moore" << std::endl;
        else std::cout << ", vonNeumann" << std::endl;
        SetupColors();  // the color palette is set based upon the number of states selected by the user
        RandomizeTexture(); // creates the initial grid of cells randomly
        CopyArrays(); // copy the values of the array to its clone
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
		SwapArrays();    // alternate using the 2 arrays for reading and writing
		for (int x = xStartIndex; x < xEndIndex; x++)
		{                                                   // for every cell (pixel) in the grid
			for (int y = yStartIndex; y < yEndIndex; y++)
			{
				if ((*curr)[x][y] == colorNumber - 1) successorIndex = 0;
				else successorIndex = (*curr)[x][y] + 1;                   // decide which the successor state is

				if (NeighbourhoodAlgorithm(x, y, successorIndex))   // check whether there are enough neighbours with the successor state around
				{
                    (*next)[x][y] = successorIndex;
					Draw(x, y, olc::Pixel(colorPalette[(*curr)[x][y]].red, colorPalette[(*curr)[x][y]].green, colorPalette[(*curr)[x][y]].blue));   // set the new value if yes
				}
			}
		}
		return true;
	}

private:
    void CopyArrays() 
    {
        for (int i = 0; i < TOT_SIZE; i++)
            for (int j = 0; j < TOT_SIZE; j++)
                m_second[i][j] = m_first[i][j];
    }

    void SwapArrays()
    {
        if (alter)
        {
            curr = &m_first;
            next = &m_second;
        }
        else
        {
            curr = &m_second;
            next = &m_first;
        }
        alter = !alter;
    }

	bool NeighbourhoodAlgorithm(int x, int y, int successorIndex)
	{
		int count = 0;

		for (int i = -range; i <= range; i++)
		{                                           // check every cell around the current, in a range-sided square
			for (int j = -range; j <= range; j++)
			{
				if (moore || ((std::abs(i) + std::abs(j)) <= range))   // if Moore is used then go ahead and check every cell in the square grid...
				{                                                   // ... if not then check if the vonNeumann distance is within range
					if ((*curr)[x+i][y+j] == successorIndex) count++; // increase the total count of successor neighbours
					if (count >= threshold) return true;    // return true if the count surpasses the threshold
				}
			}
		}
		return false;
	}

	void RandomizeTexture()
	{
		int colorIndex;
		Color color;

        // seed the random number generator with the current time
        time_t  timev;
        time(&timev);
		srand(timev);

		for (int x = 0; x < textureWidth; x++)
		{
			for (int y = 0; y < textureHeight; y++)
			{
				if (x < margin || y < margin || x >= (textureWidth - margin) || y >= (textureHeight - margin))
				{
					colorIndex = -2;
					color = { 0, 0, 0 }; // color of the semitransparent part of the frame
				}
				else if (x < xStartIndex || y < yStartIndex || x >= xEndIndex || y >= yEndIndex)
				{
					colorIndex = -1;
					color = { 255, 255, 255 };    // white color of the frame
				}
				else
				{
					colorIndex = rand() % colorNumber;  // everything else set randomly
					color = colorPalette[colorIndex];
				}
                m_first[x][y] = colorIndex;
				Draw(x, y, olc::Pixel(color.red, color.green, color.blue));
			}
		}
	}

    void SetupColors()  // i wanted each number of states to have a specific color palette...
    {                           // so i hardcoded for every single case
        switch (colorNumber)
        {
        case 18:
            colorPalette = {
            Color{181, 0, 0}, //dark red
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 157, 0}, //dark green
            Color{0, 254, 0}, //dark blue green
            Color{0, 157, 99}, //bright green blue
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{0, 0, 136}, //dark blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{216, 0, 255}, //mauve
            Color{255, 0, 181} //pink
            };
            break;
        case 17:
            colorPalette = {
            Color{181, 0, 0}, //dark red
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 254, 0}, //dark blue green
            Color{0, 157, 99}, //bright green blue
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{0, 0, 136}, //dark blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{216, 0, 255}, //mauve
            Color{255, 0, 181} //pink
            };
            break;
        case 16:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 254, 0}, //dark blue green
            Color{0, 157, 99}, //bright green blue
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{0, 0, 136}, //dark blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{216, 0, 255}, //mauve
            Color{255, 0, 181} //pink
            };
            break;
        case 15:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 254, 0}, //dark blue green
            Color{0, 157, 99}, //bright green blue
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{0, 0, 136}, //dark blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 14:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 157, 99}, //bright green blue
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{0, 0, 136}, //dark blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 13:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{0, 0, 136}, //dark blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 12:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 255, 255}, //bright blue
            Color{0, 0, 255}, //blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 11:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{185, 255, 0}, //yellow green
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 10:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{211, 46, 0}, //red - orange
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 9:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            Color{255, 0, 181} //pink
            };
            break;
        case 8:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{255, 153, 0}, //orange
            Color{250, 180, 0}, //orange - yellow
            Color{255, 255, 0}, //yellow
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            };
            break;
        case 7:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{255, 153, 0}, //orange
            Color{255, 255, 0}, //yellow
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{63, 0, 255}, //indigo
            Color{127, 0, 255}, //violet
            };
            break;
        case 6:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{255, 153, 0}, //orange
            Color{255, 255, 0}, //yellow
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{127, 0, 255} //violet
            };
            break;
        case 5:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{255, 153, 0}, //orange
            Color{0, 255, 0}, //green
            Color{0, 0, 255}, //blue
            Color{127, 0, 255} //violet
            };
            break;
        case 4:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{255, 255, 0}, //yellow
            Color{0, 255, 0}, //green
            Color{127, 0, 255} //violet
            };
            break;
        case 3:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{0, 255, 0}, //green
            Color{127, 0, 255} //violet
            };
            break;
        default:
            colorPalette = {
            Color{255, 0, 0}, //red
            Color{0, 255, 0}, //green
            };
            break;
        }
    }
};


int main()
{
    //* USE HEAP MEMORY
	CCA *app;
    app = new CCA();
	if (app->Construct(TOT_SIZE, TOT_SIZE, 1, 1))
		app->Start();
    delete(app);
    //*/

    /* USE STACK MEMORY
    CCA app;
    if (app.Construct(TOT_SIZE, TOT_SIZE, 1, 1))
        app.Start();
    //*/

	return 0;
}