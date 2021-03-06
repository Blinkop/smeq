/*
  ==============================================================================

    TimeFreqSpectrum.h
    Created: 15 Feb 2017 10:26:57pm
    Author:  Anthony

  ==============================================================================
*/

#ifndef TIMEFREQSPECTRUM_H_INCLUDED
#define TIMEFREQSPECTRUM_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Spectrum.h"
#include "MelSpectral.h"
#include <cmath>

class TimeFreqSpectrum : public Spectrum
{
public:
	TimeFreqSpectrum()
		: spectogramImage(Image::PixelFormat::ARGB, 16, 512, true),
		  oYScale(std::log10(20), std::log10(20000)),
		  oYScaleMel(MelMath::hertzToMel(20), MelMath::hertzToMel(20000))
	{
		mode = "Hz(t)";
		setOpaque(true);
	}
	~TimeFreqSpectrum()
	{

	}

	void changeSpeed(float speed)
	{
		int width = jmap(speed, static_cast<float>(16), static_cast<float>(1024));
		spectogramImage = spectogramImage.rescaled(width, 512, Graphics::ResamplingQuality::lowResamplingQuality);
		repaint();
	}

	void pause()
	{
		spectogramImage = spectogramImage.rescaled(1024, 512, Graphics::ResamplingQuality::lowResamplingQuality);
	}

	void paint(Graphics& g) override
	{
		Font mcF("Times New Roman", 10.0f, Font::bold | Font::italic);
		g.setFont(mcF);
		g.fillAll(Colours::darkgoldenrod);
		g.drawImage(spectogramImage, getLocalBounds().toFloat());
		g.drawArrow(freqLine, 1.0f, 5, 10);
		g.drawArrow(timeLine, 1.0f, 5, 10);

		g.setColour(Colours::white);
		for (int i = 0; i < 10; i++)
		{
			g.drawHorizontalLine(oYPoints[i].getY(), oYPoints[i].getX(), getWidth());
			g.drawText(String(MelMath::melToHertz(melScales[i])), oYPoints[i].getX() + 2, oYPoints[i].getY() + 5, 40, 10, Justification::centred);
		}
	}

	void resized() override
	{
		Rectangle<int> localBounds = getLocalBounds();

		freqLine.setStart(0, localBounds.getHeight());
		timeLine.setStart(freqLine.getStart());
		freqLine.setEnd(0, 0);
		timeLine.setEnd(localBounds.getWidth(), localBounds.getHeight());

		oYPoints[0].setXY(0, getHeight() - 20);
		for (int i = 1; i < 10; i++)
			oYPoints[i].setXY(0, localBounds.getHeight() - jmap(static_cast<double>(melScales[i]),
				oYScaleMel.getStart(), oYScaleMel.getEnd(), static_cast<double>(0), static_cast<double>(freqLine.getLength())));


	}

	void renderNextFrame(float* fftData, const float maxLevel)
	{
		Range<float> minMax = FloatVectorOperations::findMinAndMax(fftData, fftSize);

		const int rightEdge = spectogramImage.getWidth() - 1;
		const int imageHeight = spectogramImage.getHeight();
		int downLinePosition = imageHeight;

		spectogramImage.moveImageSection(0, 0, 1, 0, rightEdge, imageHeight);

		if (maxLevel)
		{
			for (int i = 0; i < fftSize; i++)
			{
				const int fftIndex = jmap(static_cast<int>(bandDiff * (i + 1)), 0, static_cast<int>(sampleRate / 2), 0, fftSize - 1);
				const int level = jmap(fftData[fftIndex], minMax.getStart(),
					minMax.getEnd(), 0.0f, 255.0f);

				/*const int yPosition = imageHeight
					- jmap(static_cast<double>(std::log10(bandDiff * (i + 1))),
						oYScale.getStart(), oYScale.getEnd(), static_cast<double>(0), static_cast<const double>(imageHeight));*/
				const int yPosition = imageHeight
					- jmap(static_cast<double>(MelMath::hertzToMel(bandDiff * (i + 1))),
						oYScaleMel.getStart(), oYScaleMel.getEnd(), static_cast<double>(0), static_cast<const double>(imageHeight));

				for (int j = downLinePosition; j > yPosition; j--)
					spectogramImage.setPixelAt(rightEdge, j, Colour::fromRGBA(0, 0, 0, level));
				downLinePosition = yPosition;
			}
		}
		else
		{
			for (int i = 0; i < imageHeight; i++)
				spectogramImage.setPixelAt(rightEdge, i, Colour::fromRGBA(0, 0, 0, 0));
		}

		repaint();
	}

	void prepareToRender(double sampleRate, int dataSize)
	{
		this->sampleRate = sampleRate;
		this->fftSize = dataSize / 2;

		bandDiff = 20000 / fftSize;

		spectogramImage.clear(spectogramImage.getBounds(), Colours::darkgoldenrod);
	}

private:
	Image spectogramImage;
	Line<float> freqLine;
	Line<float> timeLine;

	Point<float> oYPoints[10];
	const Range<double> oYScale;
	const Range<double> oYScaleMel;
	int freqScales[10] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
	int melScales[10] = { 32, 452, 873, 1293, 1714, 2135, 2555, 2976, 3396, 3816 };
};


#endif  // TIMEFREQSPECTRUM_H_INCLUDED
