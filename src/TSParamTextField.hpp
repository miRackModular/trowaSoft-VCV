#include "TSTextField.hpp"
#include "trowaSoftComponents.hpp"
#include <widget/Widget.hpp> //#include "widgets.hpp"

// Companion text field a for a param control.
// Should set the param value and dirty the knob.
/// TODO: Need some event notification / callback from the param control when its value changes...
struct TSParamTextField : TSTextField {
	// The param widget this text field should be attached to.
	ParamWidget* control = NULL;
	// Pointer to dirty (if control is FrameBufferWidget)
	bool* isDirty = NULL;
	// If the control needs dirty set.
	bool isBufferedCtrl = false;
	// The format string to use (if any).
	const char* formatString;
	// Translate knob value to text value.
	float(*knob2TextVal)(float) = NULL;
	// Translate text value to knob value.
	float(*text2KnobVal)(float) = NULL;
	// This field is currently being edited.
	char isEditing = 0;
	// Last control value.
	float lastControlVal = -10000;

	enum AutoHideMode {
		// Do not hide automatically.
		AutoHideOff,
		// Automatically hide this text box when it loses focus.
		AutoHideOnDefocus
	};
	// Hide this text box automatically
	AutoHideMode autoHideMode = AutoHideMode::AutoHideOff;

	//-----------------------------------------------------------------------------------------------
	// TSParamTextField()
	// @textType: (IN) Text type for this field (for validation).
	// @maxLength: (IN) Max length for this field.
	// @paramCtl: (IN) The widget for control.
	// @formatStr: (IN) The format string.
	//-----------------------------------------------------------------------------------------------
	TSParamTextField(TextType textType, int maxLength, ParamWidget* paramCtl, const char* formatStr);
	//-----------------------------------------------------------------------------------------------
	// saveValue()
	// Validate input and save value (valid values only).
	//-----------------------------------------------------------------------------------------------
	void saveValue();
	//-----------------------------------------------------------------------------------------------
	// step()
	// Set value to match the control.
	//-----------------------------------------------------------------------------------------------
	void step() override;
	//-----------------------------------------------------------------------------------------------
	// onAction()
	// Save value if valid.
	//-----------------------------------------------------------------------------------------------
	void onAction(const event::Action &e) override;
	//-----------------------------------------------------------------------------------------------
	// onFocus()
	// Set flag to not update from the control.
	//-----------------------------------------------------------------------------------------------
	void onSelect(const event::Select &e) override
	{
DEBUG("TSParamTextField::onSelect(%d) - visible = %d", id, visible);		
		if (this->visible) {
			if (!e.isConsumed())
			{
				//e.consumed = true;
				e.consume(this);
				isEditing = 3;
				this->selectAll();
				// selection = text.size();
				// cursor =  0; //text.size();
				// selection = 0;
				// cursor =  text.size();				
DEBUG("TSParamTextField::onSelect(%d). cursor = %d, selection = %d", id, cursor, selection);				
			}
		}
		return;
	}
	//-----------------------------------------------------------------------------------------------
	// onDefocus()
	// Validate input, set control value to match, format the text field number.
	//-----------------------------------------------------------------------------------------------
	void onDeselect(const event::Deselect &e) override;
	//-----------------------------------------------------------------------------------------------
	// setText()
	// @val : (IN) Float value to set the text.
	// Uses the format string.
	//-----------------------------------------------------------------------------------------------
	void setText(float val);
};