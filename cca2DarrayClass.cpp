/*
IF USING STACK MEMORY FOR THE APP THEN INCREASE STACK RESERVE SIZE TO ABOUT 3 MB
MSVC: Project > Properties > Linker > Configuration Properties > Linker > System > Stack Reserve Size: 3145728

By default, the main application will use the heap memory. That can be changed in the main function.
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <stdlib.h>

#define TOT_SIZE 580
#define INNER_SIZE 540
#define MARGIN 15
#define BORDER 5

struct Color {
	int red;
	int green;
	int blue;
};

class DoubleArray {
private:
    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> m_first = {};
    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> m_second = {};
    bool m_first_curr = true;
public:
    void copy() {
        for (int i = 0; i < TOT_SIZE; i++)
            for (int j = 0; j < TOT_SIZE; j++)
                m_second[i][j] = m_first[i][j];
    }

    void set_first(int x, int y, int value) {
        m_first[x][y] = value;
    }

    void set_curr(int x, int y, int value) {
        if (m_first_curr) {
            m_first[x][y] = value;
        }
        else {
            m_second[x][y] = value;
        }
    }

    void set_next(int x, int y, int value) {
        if (!m_first_curr) {
            m_first[x][y] = value;
        }
        else {
            m_second[x][y] = value;
        }
    }

    int get_curr(int x, int y) {
        if (m_first_curr) {
            return m_first[x][y];
        }
        else {
            return m_second[x][y];
        }
    }

    void swap_arrays() {
        m_first_curr = !m_first_curr;
    }

    std::array<std::array<int, TOT_SIZE>, TOT_SIZE> get_next() {
        if (m_first_curr) {
            return m_second;
        }
        else {
            return m_first;
        }
    }
};

class CCA : public olc::PixelGameEngine
{
private:
	int range = 6;
	int threshold = 7;
	int colorNumber = 18;
	bool moore = false;
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
    DoubleArray stateArrays; //using 2 alternaticing arrays, 1 to read from and 1 to write to

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
        stateArrays.copy(); // copy the values of the array to its clone
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
		stateArrays.swap_arrays();    // alternate using the 2 arrays for reading and writing
		for (int x = xStartIndex; x < xEndIndex; x++)
		{                                                   // for every cell (pixel) in the grid
			for (int y = yStartIndex; y < yEndIndex; y++)
			{
				if (stateArrays.get_curr(x, y) == colorNumber - 1) successorIndex = 0;
				else successorIndex = stateArrays.get_curr(x, y) + 1;                   // decide which the successor state is

				if (NeighbourhoodAlgorithm(x, y, successorIndex))   // check whether there are enough neighbours with the successor state around
				{
                    stateArrays.set_next(x, y, successorIndex);
					Draw(x, y, olc::Pixel(colorPalette[stateArrays.get_curr(x, y)].red, colorPalette[stateArrays.get_curr(x, y)].green, colorPalette[stateArrays.get_curr(x, y)].blue));   // set the new value if yes
				}
			}
		}
		return true;
	}

private:
	bool NeighbourhoodAlgorithm(int x, int y, int successorIndex)
	{
		int count = 0;

		for (int i = -range; i <= range; i++)
		{                                           // check every cell around the current, in a range-sided square
			for (int j = -range; j <= range; j++)
			{
				if (moore || ((std::abs(i) + std::abs(j)) <= range))   // if Moore is used then go ahead and check every cell in the square grid...
				{                                                   // ... if not then check if the vonNeumann distance is within range
					if (stateArrays.get_curr(x+i, y+j) == successorIndex) count++; // increase the total count of successor neighbours
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
                stateArrays.set_first(x, y, colorIndex);
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