#ifndef MYDOUBLESLIDER
#define MYDOUBLESLIDER


#include "wx/wx.h"
#include <wx/slider.h>


class wxDoubleSlider : public wxControl
{
public:

	wxDoubleSlider(wxWindow *parent,
		wxWindowID id,
		//const wxString& label,
		int leftValue, int rightValue, int minValue, int maxValue,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSL_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxDoubleSlider");

	int GetLeftValue();
	int GetRightValue();
	void SetLeftValue(int lval);
	void SetRightValue(int rval);
	void SetFloatLabel(bool f);
	wxSize DoGetBestSize();	
	void OnPaint(wxPaintEvent&);
	void OnLeftDown(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnWheel(wxMouseEvent& event);

	void SetRange(int min_val, int max_val);
	int GetMax();
	int GetMin();
	void SetRangeColor(const wxColor& c);
	void DisableRangeColor();
	void SetThumbColor(const wxColor& c1, const wxColor& c2);
	void DisableThumbColor();

protected:
	void paintNow();
	void render(wxDC& dc);
	void DrawThumb(wxDC& dc, wxCoord x, wxCoord y);
	

private:
	wxWindow* parent_;
	wxWindowID id_;
	bool horizontal_;
	int margin;
	double scale;
	int leftval, rightval, minval, maxval;
	int sel_, last_sel_;
	//int prevx, prevy;
	bool floatlabel;
	bool use_range_color_;
	wxColor range_color_;
	bool use_thumb_color_;
	wxColor thumb_color1_;
	wxColor thumb_color2_;
};


#endif
