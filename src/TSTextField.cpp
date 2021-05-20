#include "TSTextField.hpp"
#include "widgets.hpp"
#include "ui.hpp"
// for gVg
#include "window.hpp"
using namespace rack;

#include "trowaSoftComponents.hpp"

TSTextField::TSTextField(TextType textType) : TextField() {
	setTextType(textType);
	font = Font::load(assetPlugin(plugin, TROWA_MONOSPACE_FONT));
	fontSize = 14.0f;
	backgroundColor = FORMS_DEFAULT_BG_COLOR;
	color = FORMS_DEFAULT_TEXT_COLOR;
	textOffset = Vec(0, 0);
	borderWidth = 1;
	borderColor = FORMS_DEFAULT_BORDER_COLOR;
	//caretColor = COLOR_TS_RED;// nvgRGBAf(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, 0.70);
	caretColor = nvgRGBAf((color.r + backgroundColor.r) / 2.0, (color.g + backgroundColor.g) / 2.0, (color.b + backgroundColor.b) / 2.0, 0.70);
	return;
}
TSTextField::TSTextField(TextType textType, int maxLength) : TSTextField(textType) {
	this->maxLength = maxLength;
	return;
}


// Taken from Rack's LEDTextField
int TSTextField::getTextPosition(Vec mousePos) {
	bndSetFont(font->handle);
	int textPos = bndIconLabelTextPosition(gVg, textOffset.x, textOffset.y,
		box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
		-1, fontSize, displayStr.c_str(), mousePos.x, mousePos.y);
	bndSetFont(gGuiFont->handle);
	return textPos;
}

// Draw if visible.
void TSTextField::draw(NVGcontext *vg) {
	if (visible)
	{
		// Draw taken from Rack's LEDTextField and modified for scrolling (my quick & dirty ghetto text scrolling---ONLY truly effective for calculating the width with MONOSPACE font
		// since I don't want to do a bunch of calcs... [lazy]).
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);

		// Background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);

		// Border
		if (borderWidth > 0) {
			nvgStrokeWidth(vg, borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStroke(vg);
		}

		// Text
		if (font->handle >= 0) {
			bndSetFont(font->handle);

			//NVGcolor highlightColor = color;
			//highlightColor.a = 0.5;
			int begin = min(cursor, selection);
			int end = (this == gFocusedWidget) ? max(cursor, selection) : -1;

			// Calculate overflow and the displayed text (based on bounding box)
			// Currently the scrolling should work for any font, **BUT** the width calculation is only really good for monospace.
			float txtBounds[4] = { 0,0,0,0 };
			nvgTextAlign(vg, NVG_ALIGN_LEFT);
			nvgFontSize(vg, fontSize);
			nvgFontFaceId(vg, font->handle);
			int maxTextWidth = box.size.x - textOffset.x * 2 - fontSize / 2.0; // There should be a caret
			float estLetterSize = nvgTextBounds(vg, 0, 0, "X", NULL, txtBounds); // Estimate size of a letter (accurate for monospace)
			float nextX = nvgTextBounds(vg, 0, 0, text.c_str(), NULL, txtBounds); // Calculate full string size

			displayStr = text;
			if (nextX > maxTextWidth) {
				int nChars = maxTextWidth / estLetterSize - 1;
				if (nChars < 1)
					nChars = 1;

				if (this == gFocusedWidget) {
					int lastIx = (cursor > nChars) ? cursor : nChars;
					int startIx = clamp(lastIx - nChars, 0, lastIx);
					displayStr = text.substr(startIx, nChars);
					begin -= startIx;
					if (end > -1)
						end -= startIx;
				}
				else {
					displayStr = text.substr(0, nChars);
				}
			}


			// The caret color actually isn't the cursor color (that is hard-coded as nvgRGBf(0.337,0.502,0.761))
			// 

			//void bndIconLabelCaret(NVGcontext *ctx, float x, float y, float w, float h,
			//	int iconid, NVGcolor color, float fontsize, const char *label,
			//	NVGcolor caretcolor, int cbegin, int cend
			bndIconLabelCaret(vg, /*x*/ textOffset.x, /*y*/ textOffset.y,
				/*w*/ box.size.x - 2 * textOffset.x, /*h*/ box.size.y - 2 * textOffset.y,
				/*iconid*/ -1, /*textColor*/ color, /*fontsize*/ fontSize, 
				/*label*/ displayStr.c_str(), 
				/*caretcolor*/ caretColor, /*cbegin*/ begin, /*cend*/ end);

			bndSetFont(gGuiFont->handle);
		}

		nvgResetScissor(vg);
	}
} // end draw()

// Remove invalid chars from input.
std::string TSTextField::cleanseString(std::string newText)
{
	if (allowedTextType == TextType::Any)
	{
		return newText.substr(0, maxLength);
	}
	else
	{
		// Remove invalid chars
		std::stringstream cleansedStr;
		// Issue: https://github.com/j4s0n-c/trowaSoft-VCV/issues/5. Changed from string constant (emtpy string "") to string object empty string ("") to older Linux compilers. Thx to @Chaircrusher.
		std::regex_replace(std::ostream_iterator<char>(cleansedStr), newText.begin(), newText.end(), regexInvalidChar, std::string(""));
		return cleansedStr.str().substr(0, maxLength);
	}
} // end cleanseString()

// Remove invalid chars
/** Inserts text at the cursor, replacing the selection if necessary */
void TSTextField::insertText(std::string newText) {
	if (cursor != selection) {
		int begin = min(cursor, selection);
		this->text.erase(begin, std::abs(selection - cursor));
		cursor = selection = begin;
	}
	std::string cleansedStr = cleanseString(newText);
	this->text.insert(cursor, cleansedStr);
	cursor += cleansedStr.size();
	selection = cursor;
	onTextChange();
	return;
} // end insertText()

void TSTextField::setText(std::string text) {
	this->text = cleanseString(text);
	selection = cursor = text.size();
	onTextChange();
}
// When the text changes.
void TSTextField::onTextChange() {
	text = cleanseString(text);
	cursor = clamp(cursor, 0, text.size());
	selection = clamp(selection, 0, text.size());
	//debug("onTextChange() - New cursor: %d", cursor);
	return;
} // end onTextChanged()
