/** FXScintilla source code edit control
 *
 *  PlatFOX.cxx - implementation of platform facilities on the FOX toolkit
 *
 *  Copyright 2001-2003 by Gilles Filippini <gilles.filippini@free.fr>
 *
 *  Adapted from the Scintilla source PlatGTK.cxx 
 *  Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
 *
 *  ====================================================================
 *
 *  This file is part of FXScintilla.
 * 
 *  FXScintilla is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  FXScintilla is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with FXScintilla; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **/

extern "C" {
# include "../ltdl/ltdl.h"
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
# if defined(__CYGWIN__) || defined(__MINGW32__)
#  include <windows.h>
#  ifdef PIC
#   define FOXDLL
#  endif
# endif
# include <sys/time.h>
# include <fox/fx.h>
# include <fox/fxkeys.h>
#else
# include <time.h>
# include <windows.h>
# include <fx.h>
# include <fxkeys.h>
#endif	// !defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)

#include "Platform.h"

#include "Scintilla.h"
#include "ScintillaWidget.h"

#if HAVE_FOX_1_1
#define getRoot getRootWindow
#define setTextFont setFont
#endif

#include <map>
using namespace std;

// X has a 16 bit coordinate space, so stop drawing here to avoid wrapping
static const int maxCoordinate = 32000;

Point Point::FromLong(long lpoint) {
	return Point(
		Platform::LowShortFromLong(lpoint), 
		Platform::HighShortFromLong(lpoint));
}

Palette::Palette() {
	used = 0;
	allowRealization = false;
	visual = 0;
}

Palette::~Palette() {
	Release();
}

void Palette::Release() {
	used = 0;
}

// This method either adds a colour to the list of wanted colours (want==true)
// or retrieves the allocated colour back to the ColourPair.
// This is one method to make it easier to keep the code for wanting and retrieving in sync.
void Palette::WantFind(ColourPair &cp, bool want) {
	if (want) {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired)
				return;
		}
	
		if (used < numEntries) {
			entries[used].desired = cp.desired;
			entries[used].allocated.Set(cp.desired.AsLong());
			used++;
		}
	} else {
		for (int i=0; i < used; i++) {
			if (entries[i].desired == cp.desired) {
				cp.allocated = entries[i].allocated;
				return;
			}
		}
		cp.allocated.Set(cp.desired.AsLong());
	}
}

void Palette::Allocate(Window & /* w */) {
// <FIXME/>
}

Font::Font() : id(0) {}

Font::~Font() {}

#ifndef WIN32

static const char *CharacterSetName(int characterSet) {
	switch (characterSet) {
	case SC_CHARSET_ANSI:
		return "iso8859-*";
	case SC_CHARSET_DEFAULT:
		return "iso8859-*";
	case SC_CHARSET_BALTIC:
		return "*-*";
	case SC_CHARSET_CHINESEBIG5:
		return "*-*";
	case SC_CHARSET_EASTEUROPE:
		return "*-2";
	case SC_CHARSET_GB2312:
		return "gb2312.1980-*";
	case SC_CHARSET_GREEK:
		return "*-7";
	case SC_CHARSET_HANGUL:
		return "ksc5601.1987-*";
	case SC_CHARSET_MAC:
		return "*-*";
	case SC_CHARSET_OEM:
		return "*-*";
	case SC_CHARSET_RUSSIAN:
		return "*-r";
	case SC_CHARSET_SHIFTJIS:
		return "jisx0208.1983-*";
	case SC_CHARSET_SYMBOL:
		return "*-*";
	case SC_CHARSET_TURKISH:
		return "*-9";
	case SC_CHARSET_JOHAB:
		return "*-*";
	case SC_CHARSET_HEBREW:
		return "*-8";
	case SC_CHARSET_ARABIC:
		return "*-6";
	case SC_CHARSET_VIETNAMESE:
		return "*-*";
	case SC_CHARSET_THAI:
		return "*-1";
	default:
		return "*-*";
	}
}

void Font::Create(const char *faceName, int characterSet,
	int size, bool bold, bool italic) {
	Release();
	// If name of the font begins with a '-', assume, that it is
	// a full fontspec.
	if (faceName[0] == '-'){
		id = new FXFont(FXApp::instance(), faceName);
		if (id)
			return;
	}
	char fontspec[300];
	fontspec[0] = '\0';
	strcat(fontspec, "-*-");
	strcat(fontspec, faceName);
	if (bold)
		strcat(fontspec, "-bold");
	else
		strcat(fontspec, "-medium");
	if (italic)
		strcat(fontspec, "-i");
	else
		strcat(fontspec, "-r");
	strcat(fontspec, "-*-*-*");
	char sizePts[100];
	sprintf(sizePts, "-%0d", size * 10);
	strcat(fontspec, sizePts);
	strcat(fontspec, "-*-*-*-*-");
	strcat(fontspec, CharacterSetName(characterSet));
	id = new FXFont(FXApp::instance(), fontspec);
	if (!id) {
		// Font not available so substitute with the app default font.
		id = FXApp::instance()->getNormalFont();
	}
	if (id)
		id->create();
}

#else // WIN32

static int CharacterSetCode(int characterSet) {
	switch (characterSet) {
	case SC_CHARSET_ANSI:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_DEFAULT:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_BALTIC:
		return FONTENCODING_BALTIC;
	case SC_CHARSET_CHINESEBIG5:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_EASTEUROPE:
		return FONTENCODING_EASTEUROPE;
	case SC_CHARSET_GB2312:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_GREEK:
		return FONTENCODING_GREEK;
	case SC_CHARSET_HANGUL:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_MAC:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_OEM:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_RUSSIAN:
		return FONTENCODING_RUSSIAN;
	case SC_CHARSET_SHIFTJIS:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_SYMBOL:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_TURKISH:
		return FONTENCODING_TURKISH;
	case SC_CHARSET_JOHAB:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_HEBREW:
		return FONTENCODING_HEBREW;
	case SC_CHARSET_ARABIC:
		return FONTENCODING_ARABIC;
	case SC_CHARSET_VIETNAMESE:
		return FONTENCODING_DEFAULT;
	case SC_CHARSET_THAI:
		return FONTENCODING_THAI;
	default:
		return FONTENCODING_DEFAULT;
	}
}

void Font::Create(const char *faceName, int characterSet,
	int size, bool bold, bool italic) {
	Release();
	id = new FXFont(FXApp::instance(), faceName, size,
									bold ? FONTWEIGHT_BOLD : FONTWEIGHT_NORMAL,
									italic ? FONTSLANT_ITALIC : FONTSLANT_REGULAR,
									CharacterSetCode(characterSet));
	if (!id) {
		// Font not available so substitute with the app default font.
		id = FXApp::instance()->getNormalFont();
	}
	if (id)
		id->create();
}

#endif // WIN32

void Font::Release() {
	if (id)
		delete id;
	id = 0;
}

// ====================================================================
// Surface
// ====================================================================

class SurfaceImpl : public Surface {
	bool unicodeMode;
	FXDrawable *drawable;
	FXImage *ppixmap;
	static SurfaceImpl *s_dc_owner;
	FXDCWindow *_dc;
	FXDCWindow *dc();
	int x;
	int y;
	bool inited;
	bool createdDC;
public:
	SurfaceImpl();
	virtual ~SurfaceImpl();

	void Init(WindowID wid);
	void Init(SurfaceID sid, WindowID wid);
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid);

	void Release();
	bool Initialised();
	void PenColour(ColourAllocated fore);
	void BackColour(ColourAllocated back);
	int LogPixelsY();
	int DeviceHeightFont(int points);
	void MoveTo(int x_, int y_);
	void LineTo(int x_, int y_);
	void Polygon(Point *pts, int npts, ColourAllocated fore, ColourAllocated back);
	void RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void FillRectangle(PRectangle rc, ColourAllocated back);
	void FillRectangle(PRectangle rc, Surface &surfacePattern);
	void RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back);
	void Copy(PRectangle rc, Point from, Surface &surfaceSource);

	void DrawTextBase(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore);
	void DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore, ColourAllocated back);
	void DrawTextTransparent(PRectangle rc, Font &font_, int ybase, const char *s, int len, ColourAllocated fore);
	void MeasureWidths(Font &font_, const char *s, int len, int *positions);
	int WidthText(Font &font_, const char *s, int len);
	int WidthChar(Font &font_, char ch);
	int Ascent(Font &font_);
	int Descent(Font &font_);
	int InternalLeading(Font &font_);
	int ExternalLeading(Font &font_);
	int Height(Font &font_);
	int AverageCharWidth(Font &font_);

	int SetPalette(Palette *pal, bool inBackGround);
	void SetClip(PRectangle rc);
	void FlushCachedState();

	void SetUnicodeMode(bool unicodeMode_);
	virtual void SetDBCSMode(int /* codePage */) {}
};

SurfaceImpl * SurfaceImpl::s_dc_owner = NULL;

SurfaceImpl::SurfaceImpl() : unicodeMode(false), drawable(0), ppixmap(0), _dc(0),
x(0), y(0), inited(false) {}


SurfaceImpl::~SurfaceImpl() {
	Release();
}

FXDCWindow * SurfaceImpl::dc()
{
	if (s_dc_owner != this) {
		if (s_dc_owner) {
			delete s_dc_owner->_dc;
			s_dc_owner->_dc = NULL;
		}
		s_dc_owner = this;
		_dc = (drawable) ? new FXDCWindow(drawable) : NULL;
	}
	return _dc;
		
}

void SurfaceImpl::Release() {
	drawable = 0;
	if (_dc) {
		delete _dc;
		_dc = 0;
		s_dc_owner = 0;
	}
	if (ppixmap)
		delete ppixmap;
	ppixmap = 0;
	x = 0;
	y = 0;
	createdDC = false;
	inited = false;
}

bool SurfaceImpl::Initialised() {
	return inited;
}

void SurfaceImpl::Init(WindowID) {
	Release();
	inited = true;
}

void SurfaceImpl::Init(SurfaceID sid, WindowID) {
	Release();
	drawable = reinterpret_cast<FXDrawable *>(sid);
	createdDC = true;
	inited = true;
}

void SurfaceImpl::InitPixMap(int width, int height, Surface*, WindowID) {
	Release();
	if (height > 0 && width > 0)
		ppixmap = new FXImage(FXApp::instance(), NULL, 0, width, height);
	else
		ppixmap = NULL;
	drawable = ppixmap;
	if (drawable)
		drawable->create();
	createdDC = true;
	inited = true;
}

void SurfaceImpl::PenColour(ColourAllocated fore) {
	if (dc()) {
		ColourDesired cd(fore.AsLong());
		_dc->setForeground(FXRGB(cd.GetRed(), cd.GetGreen(), cd.GetBlue()));
	}
}

void SurfaceImpl::BackColour(ColourAllocated back) {
	if (dc()) {
		ColourDesired cd(back.AsLong());
		_dc->setBackground(FXRGB(cd.GetRed(), cd.GetGreen(), cd.GetBlue()));
	}
}

int SurfaceImpl::LogPixelsY() {
	return 72;
}

int SurfaceImpl::DeviceHeightFont(int points) {
	int logPix = LogPixelsY();
	return (points * logPix + logPix / 2) / 72;
}

void SurfaceImpl::MoveTo(int x_, int y_) {
	x = x_;
	y = y_;
}

void SurfaceImpl::LineTo(int x_, int y_) {
	if (dc()) {
		_dc->drawLine(x, y, x_, y_);
	}
	x = x_;
	y = y_;
}

void SurfaceImpl::Polygon(Point *pts, int npts, ColourAllocated fore,
                      ColourAllocated back) {
	if (dc()) {
		FXPoint gpts[20];
		if (npts < static_cast<int>((sizeof(gpts)/sizeof(gpts[0])))) {
			for (int i=0;i<npts;i++) {
				gpts[i].x = pts[i].x;
				gpts[i].y = pts[i].y;
			}
			gpts[npts].x = pts[0].x;
			gpts[npts].y = pts[0].y;
			PenColour(back);
			_dc->fillPolygon(gpts, npts);
			PenColour(fore);
			_dc->drawLines(gpts, npts + 1);
		}
	}
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (dc()) {
		PenColour(fore);
		BackColour(back);
		_dc->drawRectangle(rc.left, rc.top,
		                  rc.right - rc.left + 1, rc.bottom - rc.top + 1);
	}
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourAllocated back) {
	if (dc() && (rc.left < maxCoordinate)) {	// Protect against out of range
		// GTK+ rectangles include their lower and right edges
		rc.bottom--;
		rc.right--;
		PenColour(back);
		_dc->fillRectangle(rc.left, rc.top,
					  rc.right - rc.left + 1, rc.bottom - rc.top + 1);
	}
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	if (static_cast<SurfaceImpl &>(surfacePattern).drawable) {
		if (dc()) {
			// Tile pattern over rectangle
			// Currently assumes 8x8 pattern
			int widthPat = 8;
			int heightPat = 8;
			for (int xTile = rc.left; xTile < rc.right; xTile += widthPat) {
				int widthx = (xTile + widthPat > rc.right) ? rc.right - xTile : widthPat;
				for (int yTile = rc.top; yTile < rc.bottom; yTile += heightPat) {
					int heighty = (yTile + heightPat > rc.bottom) ? rc.bottom - yTile : heightPat;
					_dc->drawArea(static_cast<SurfaceImpl &>(surfacePattern).drawable,
												0, 0,
												widthx, heighty,
												xTile, yTile);
				}
			}
		}
	} else {
		// Something is wrong so try to show anyway
		// Shows up black because colour not allocated
		FillRectangle(rc, ColourAllocated(0));
	}
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (((rc.right - rc.left) > 4) && ((rc.bottom - rc.top) > 4)) {
		// Approximate a round rect with some cut off corners
		Point pts[] = {
		    Point(rc.left + 2, rc.top),
		    Point(rc.right - 2, rc.top),
		    Point(rc.right, rc.top + 2),
		    Point(rc.right, rc.bottom - 2),
		    Point(rc.right - 2, rc.bottom),
		    Point(rc.left + 2, rc.bottom),
		    Point(rc.left, rc.bottom - 2),
		    Point(rc.left, rc.top + 2),
		};
		Polygon(pts, sizeof(pts) / sizeof(pts[0]), fore, back); 
	} else {
		RectangleDraw(rc, fore, back);
	}
}

void SurfaceImpl::Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back) {
	if (dc()) {
		PenColour(back);
		_dc->fillArc(rc.left, rc.top,
			    rc.right - rc.left, rc.bottom - rc.top,
			    0, 32767);
		PenColour(fore);
		_dc->drawArc(rc.left, rc.top,
			    rc.right - rc.left, rc.bottom - rc.top,
			    0, 32767);
	}
}

void SurfaceImpl::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	if (dc() && static_cast<SurfaceImpl &>(surfaceSource).drawable) {
		_dc->drawArea(static_cast<SurfaceImpl &>(surfaceSource).drawable,
		             from.x, from.y,
		             rc.right - rc.left, rc.bottom - rc.top ,
		             rc.left, rc.top);
	}
}

void SurfaceImpl::DrawTextBase(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                       ColourAllocated fore) {
	if (dc()) {
		PenColour(fore);
		_dc->setTextFont(font_.GetID());

		const int segmentLength = 1000;
		int x = rc.left;
		while ((len > 0) && (x < maxCoordinate)) {
			int lenDraw = Platform::Minimum(len, segmentLength);
			_dc->drawText(x, ybase, s, lenDraw);
			len -= lenDraw;
			if (len > 0) {
				x += font_.GetID()->getTextWidth(s, lenDraw);
			}
			s += lenDraw;
		}
	}
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                       ColourAllocated fore, ColourAllocated back) {
	if (dc()) {
		FillRectangle(rc, back);
		DrawTextBase(rc, font_, ybase, s, len, fore);
	}
}

// On GTK+, exactly same as DrawText NoClip
// <FIXME> what about FOX ? </FIXME>
void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s, int len,
                       ColourAllocated fore, ColourAllocated back) {
	DrawTextNoClip(rc, font_, ybase, s, len, fore, back);
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font_, int ybase, const char *s, int len,
																			ColourAllocated fore) {
	DrawTextBase(rc, font_, ybase, s, len, fore);
}

void SurfaceImpl::MeasureWidths(Font &font_, const char *s, int len, int *positions) {
	if (font_.GetID()) {
		int totalWidth = 0;
		for (int i=0;i<len;i++) {
			int width = font_.GetID()->getTextWidth(s + i, 1);
			totalWidth += width;
			positions[i] = totalWidth;
		}
	}
	else {
		for (int i=0;i<len;i++) {
			positions[i] = i + 1;
		}
	}
}

int SurfaceImpl::WidthText(Font &font_, const char *s, int len) {
	if (font_.GetID())
		return font_.GetID()->getTextWidth(s, len);
	else
		return 1;
}

int SurfaceImpl::WidthChar(Font &font_, char ch) {
	if (font_.GetID())
		return font_.GetID()->getTextWidth(&ch, 1);
	else
		return 1;
}

int SurfaceImpl::Ascent(Font &font_) {
	if (!font_.GetID())
		return 1;
	return font_.GetID()->getFontAscent();
}

int SurfaceImpl::Descent(Font &font_) {
	if (!font_.GetID())
		return 1;
	return font_.GetID()->getFontDescent();
}

int SurfaceImpl::InternalLeading(Font &) {
	return 0;
}

int SurfaceImpl::ExternalLeading(Font &) {
	return 0;
}

int SurfaceImpl::Height(Font &font_) {
	if (!font_.GetID())
		return 1;
	return font_.GetID()->getFontHeight();
}

int SurfaceImpl::AverageCharWidth(Font &font_) {
	if (font_.GetID())
		return font_.GetID()->getTextWidth("n", 1);
	else
		return 1;
}

int SurfaceImpl::SetPalette(Palette *, bool) {
	// Handled in palette allocation for GTK so this does nothing
// <FIXME> What about FOX ? </FIXME>
	return 0;
}

void SurfaceImpl::SetClip(PRectangle rc) {
	if (dc())
		_dc->setClipRectangle(rc.left, rc.top,
	                     rc.right - rc.left, rc.bottom - rc.top);
}

void SurfaceImpl::FlushCachedState() {}

void SurfaceImpl::SetUnicodeMode(bool unicodeMode_) {
	unicodeMode=unicodeMode_;
}

Surface *Surface::Allocate() {
	return new SurfaceImpl;
}

Window::~Window() {}

void Window::Destroy() {
	if (id)
		delete id;
	id = 0;
}

bool Window::HasFocus() {
	return id->hasFocus();
}

PRectangle Window::GetPosition() {
	// Before any size allocated pretend its 1000 wide so not scrolled
	PRectangle rc(0, 0, 1000, 1000);
	if (id) {
		rc.left = id->getX();
		rc.top = id->getY();
		rc.right = rc.left + id->getWidth();
		rc.bottom = rc.top + id->getHeight();
	}
	return rc;
}

void Window::SetPosition(PRectangle rc) {
	id->position(rc.left, rc.top, rc.Width(), rc.Height());
}


void Window::SetPositionRelative(PRectangle rc, Window relativeTo) {
	int ox = relativeTo.GetID()->getX() + rc.left;
	int oy = relativeTo.GetID()->getY() + rc.top;
	if (ox < 0)
		ox = 0;
	if (oy < 0)
		oy = 0;
	id->position(ox, oy, rc.Width(), rc.Height());
}

PRectangle Window::GetClientPosition() {
	// On GTK+, the client position is the window position
	return PRectangle(0, 0, (id) ? id->getWidth() - 1 : 1000, (id) ? id->getHeight() -1 : 1000);
}

void Window::Show(bool show) {
	if (show) {
		id->show();
		id->raise();
	}
	else
		id->hide();
}

void Window::InvalidateAll() {
	if (id) {
		id->update();
	}
}

void Window::InvalidateRectangle(PRectangle rc) {
	if (id)
		id->update(rc.left, rc.top, rc.right - rc.left + 1, rc.bottom - rc.top + 1);
}

void Window::SetFont(Font &) {
	// TODO
}

void Window::SetCursor(Cursor curs) {
	// We don't set the cursor to same value numerous times under FOX because
	// it stores the cursor in the window once it's set
	if (curs == cursorLast)
		return;
	FXDefaultCursor cursorID;
	cursorLast = curs;

	switch (curs) {
	case cursorText:
		cursorID = DEF_TEXT_CURSOR;
		break;
	case cursorArrow:
		cursorID = DEF_ARROW_CURSOR;
		break;
	case cursorUp:
		cursorID = DEF_MOVE_CURSOR;
		break;
	case cursorWait:
		cursorID = DEF_SWATCH_CURSOR;
		break;
	case cursorHand:
		// <FIXME/> Should be a hand cursor...
		cursorID = DEF_CROSSHAIR_CURSOR;
		break;
	case cursorReverseArrow:
		cursorID = DEF_RARROW_CURSOR;
		break;
	default:
		cursorID = DEF_ARROW_CURSOR;
		cursorLast = cursorArrow;
		break;
	}
	id->setDefaultCursor(id->getApp()->getDefaultCursor(cursorID));
}

void Window::SetTitle(const char *s) {
	static_cast<FXTopWindow *>(id)->setTitle(s);
}

// ====================================================================
// ListBoxFox
// ====================================================================

class ListBoxFox : public ListBox
{
	FXList * list;
	map<int, FXXPMIcon *> * pixhash;
	int desiredVisibleRows;
	unsigned int maxItemCharacters;
	unsigned int aveCharWidth;
public:
	CallBackAction doubleClickAction;
	void *doubleClickActionData;

	ListBoxFox() : list(0), pixhash(NULL), desiredVisibleRows(5), maxItemCharacters(0),
		doubleClickAction(NULL), doubleClickActionData(NULL) {
	}
	virtual ~ListBoxFox() {
		ClearRegisteredImages();
	}
	virtual void Show(bool show=true);
	virtual void SetFont(Font &font);
	virtual void Create(Window &parent, int ctrlID, int lineHeight_, bool unicodeMode_);
	virtual void SetAverageCharWidth(int width);
	virtual void SetVisibleRows(int rows);
	virtual PRectangle GetDesiredRect();
	virtual int CaretFromEdge();
	virtual void Clear();
	virtual void Append(char *s, int type = -1);
	virtual int Length();
	virtual void Select(int n);
	virtual int GetSelection();
	virtual int Find(const char *prefix);
	virtual void GetValue(int n, char *value, int len);
	virtual void Sort();
	virtual void RegisterImage(int type, const char *xpm_data);
	virtual void ClearRegisteredImages();
	virtual void SetDoubleClickAction(CallBackAction action, void *data) {
		doubleClickAction = action;
		doubleClickActionData = data;
	}
};


static int sListSortFunction(const FXListItem* item1, const FXListItem* item2) {
	return compare(item1->getText(), item2->getText());
}

class PopupListBox : public FXPopup
{
FXDECLARE(PopupListBox)
protected:
	PopupListBox() {}
protected:
	ListBoxFox * listBox;
	FXList * list;
public:
	enum {
		ID_LIST = FXPopup::ID_LAST,
		ID_LAST,
	};
public:
	long onKeyPress(FXObject *, FXSelector, void *);
	long onListKeyPress(FXObject *, FXSelector, void *);
	long onDoubleClicked(FXObject *, FXSelector, void *);
public:
	PopupListBox(FXComposite * parent, ListBoxFox * lb);
	FXList * getList() { return list; }
	virtual void setFocus() {
		FXPopup::setFocus();
		list->grabKeyboard();
	}
	virtual void killFocus() {
		list->ungrabKeyboard();
		FXPopup::killFocus();
	}
};

FXDEFMAP(PopupListBox) PopupListBoxMap[]={
  FXMAPFUNC(SEL_KEYPRESS, 0, PopupListBox::onKeyPress),
  FXMAPFUNC(SEL_KEYPRESS, PopupListBox::ID_LIST, PopupListBox::onListKeyPress),
  FXMAPFUNC(SEL_DOUBLECLICKED, 0, PopupListBox::onDoubleClicked),
};

FXIMPLEMENT(PopupListBox,FXPopup,PopupListBoxMap,ARRAYNUMBER(PopupListBoxMap))

PopupListBox::PopupListBox(FXComposite * parent, ListBoxFox * lb) :
	FXPopup(parent), listBox(lb)
{
	list = new FXList(this, 0, this, ID_LIST, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y|SCROLLERS_TRACK|HSCROLLER_NEVER);
	list->setSortFunc(sListSortFunction);
}

long PopupListBox::onKeyPress(FXObject * sender, FXSelector sel, void * ptr)
{
	return FXPopup::onKeyPress(sender, sel, ptr);
}

long PopupListBox::onListKeyPress(FXObject * sender, FXSelector sel, void * ptr)
{
	list->setTarget(NULL);
	list->onKeyPress(sender, sel, ptr);
	FXEvent * event = (FXEvent *)ptr;

  switch(event->code) {
    case KEY_Page_Up:
    case KEY_KP_Page_Up:
    case KEY_Page_Down:
    case KEY_KP_Page_Down:
    case KEY_Up:
    case KEY_KP_Up:
    case KEY_Down:
    case KEY_KP_Down:
    case KEY_Home:
    case KEY_KP_Home:
    case KEY_End:
    case KEY_KP_End:
			break;
		default:
			getOwner()->handle(this, MKUINT(0, SEL_KEYPRESS), ptr);
	}
	list->setTarget(this);
	return 1;
}

long PopupListBox::onDoubleClicked(FXObject *, FXSelector, void *)
{
	if (listBox->doubleClickAction) {
		listBox->doubleClickAction(listBox->doubleClickActionData);
	}
	return 1;
}

// ====================================================================

void ListBoxFox::Create(Window & parent, int, int, bool) {
	id = new PopupListBox(static_cast<FXComposite *>(parent.GetID()), this);
	id->create();
	list = (static_cast<PopupListBox *>(id))->getList();
}

void ListBoxFox::SetFont(Font &scint_font) {
	list->setFont(scint_font.GetID());
}

void ListBoxFox::SetAverageCharWidth(int width) {
    aveCharWidth = width;
}

void ListBoxFox::SetVisibleRows(int rows) {
	list->setNumVisible(rows);
}

PRectangle ListBoxFox::GetDesiredRect() {
	// Before any size allocated pretend its 100 wide so not scrolled
	PRectangle rc(0, 0, 100, 100);
	if (id) {
		// Height
		int rows = Length();
		if ((rows == 0) || (rows > desiredVisibleRows))
			rows = desiredVisibleRows;
		list->setNumVisible(rows);
		rc.bottom = id->getHeight();
		// Width
		int width = maxItemCharacters;
		if (width < 12)
			width = 12;
		rc.right = width * (aveCharWidth+aveCharWidth/3);
		if (Length() > rows)
			rc.right += list->verticalScrollbar()->getWidth();

		// <FIXME/>
/*		int rows = Length();
		if ((rows == 0) || (rows > desiredVisibleRows))
			rows = desiredVisibleRows;
		
		GtkRequisition req;
		int height;

		// First calculate height of the clist for our desired visible row count otherwise it tries to expand to the total # of rows
		height = (rows * GTK_CLIST(list)->row_height
		          + rows + 1
		          + 2 * (list->style->klass->ythickness 
		                 + GTK_CONTAINER(list)->border_width));
		gtk_widget_set_usize(GTK_WIDGET(list), -1, height);

		// Get the size of the scroller because we set usize on the window
		gtk_widget_size_request(GTK_WIDGET(scroller), &req);
		rc.right = req.width;
		rc.bottom = req.height;
                
		gtk_widget_set_usize(GTK_WIDGET(list), -1, -1);
		int width = maxItemCharacters;
		if (width < 12)
			width = 12;
		rc.right = width * (aveCharWidth+aveCharWidth/3);
		if (Length() > rows)
			rc.right = rc.right + 16;*/
	}
	return rc;
    
}

void ListBoxFox::Show(bool show) {
	if (show) {
		(static_cast<FXPopup *>(id))->popup(NULL,
																				id->getX(), id->getY(),
																				id->getWidth(), id->getHeight());
		list->selectItem(0);
	}
}

int ListBoxFox::CaretFromEdge() {
	// <FIXME/> return 4 + GetWidth();
	return 0;
}

void ListBoxFox::Clear() {
	list->clearItems();
	maxItemCharacters = 0;
}

void ListBoxFox::Append(char *s, int type) {
	FXXPMIcon * icon = NULL;
	if ((type >= 0) && pixhash) {
		map<int, FXXPMIcon *>::iterator it = pixhash->find(type);
		if (it != pixhash->end())
			icon = (*it).second;
	}
	list->appendItem(s, icon);
	size_t len = strlen(s);
	if (maxItemCharacters < len)
        	maxItemCharacters = len;
	if (list->getNumItems() <= desiredVisibleRows)
		list->setNumVisible(list->getNumItems());
}

int ListBoxFox::Length() {
	if (id)
		return list->getNumItems();
	return 0;
}

void ListBoxFox::Select(int n) {
	list->setCurrentItem(n, true);
}

int ListBoxFox::GetSelection() {
	return list->getCurrentItem();
}

int ListBoxFox::Find(const char *prefix) {
	int count = Length();
	for (int i = 0; i < count; i++) {
		FXString text = list->getItemText(i);
		const char* s = text.text();
		if (s && (0 == strncmp(prefix, s, strlen(prefix)))) {
			return i;
		}
	}
	return - 1;
}

void ListBoxFox::GetValue(int n, char *value, int len) {
	FXString text = list->getItemText(n);
	if (text.length() && len > 0) {
		strncpy(value, text.text(), len);
		value[len - 1] = '\0';
	} else {
		value[0] = '\0';
	}
}

void ListBoxFox::Sort() {
	list->sortItems();
}

void ListBoxFox::RegisterImage(int type, const char *xpm_data)
{
	FXXPMIcon * icon = new FXXPMIcon(FXApp::instance(), &xpm_data);
	icon->create();
	if (!pixhash)
		pixhash = new map<int, FXXPMIcon *>;
	FXXPMIcon * old = (*pixhash)[type];
	if (old)
		delete old;
	(*pixhash)[type] = icon;
}

void ListBoxFox::ClearRegisteredImages()
{
	if (pixhash) {
		map<int, FXXPMIcon *>::iterator it;
		for (it = pixhash->begin(); it != pixhash->end(); it++) {
			delete (*it).second;
		}
		delete pixhash;
	}	
}

// ====================================================================
// ListBox
// ====================================================================

ListBox::ListBox()
{
}

ListBox::~ListBox()
{
}

ListBox * ListBox::Allocate()
{
	return new ListBoxFox();
}

// ====================================================================
// Menu
// ====================================================================

Menu::Menu() : id(0) {}


void Menu::CreatePopUp() {
	Destroy();
	id = new FXMenuPane(FXApp::instance()->getCursorWindow());
}

void Menu::Destroy() {
	if (id)
		delete id;
	id = 0;
}

void Menu::Show(Point pt, Window &) {
	int screenHeight = FXApp::instance()->getRoot()->getDefaultHeight();
	int screenWidth = FXApp::instance()->getRoot()->getDefaultWidth();
	id->create();
	if ((pt.x + id->getWidth()) > screenWidth) {
		pt.x = screenWidth - id->getWidth();
	}
	if ((pt.y + id->getHeight()) > screenHeight) {
		pt.y = screenHeight - id->getHeight();
	}
	id->popup(NULL, pt.x - 4, pt.y);
  FXApp::instance()->runModalWhileShown(id);
}

#ifndef WIN32

ElapsedTime::ElapsedTime() {
	timeval curTime;
	gettimeofday(&curTime, NULL);
	bigBit = curTime.tv_sec;
	littleBit = curTime.tv_usec;
}

double ElapsedTime::Duration(bool reset) {
	timeval curTime;
	gettimeofday(&curTime, NULL);
	long endBigBit = curTime.tv_sec;
	long endLittleBit = curTime.tv_usec;
	double result = 1000000.0 * (endBigBit - bigBit);
	result += endLittleBit - littleBit;
	result /= 1000000.0;
	if (reset) {
		bigBit = endBigBit;
		littleBit = endLittleBit;
	}
	return result;
}

#else	// WIN32

static bool initialisedET = false;
static bool usePerformanceCounter = false;
static LARGE_INTEGER frequency;

ElapsedTime::ElapsedTime() {
	if (!initialisedET) {
		usePerformanceCounter = ::QueryPerformanceFrequency(&frequency);
		initialisedET = true;
	}
	if (usePerformanceCounter) {
		LARGE_INTEGER timeVal;
		::QueryPerformanceCounter(&timeVal);
		bigBit = timeVal.HighPart;
		littleBit = timeVal.LowPart;
	} else {
		bigBit = clock();
	}
}

double ElapsedTime::Duration(bool reset) {
	double result;
	long endBigBit;
	long endLittleBit;

	if (usePerformanceCounter) {
		LARGE_INTEGER lEnd;
		::QueryPerformanceCounter(&lEnd);
		endBigBit = lEnd.HighPart;
		endLittleBit = lEnd.LowPart;
		LARGE_INTEGER lBegin;
		lBegin.HighPart = bigBit;
		lBegin.LowPart = littleBit;
		double elapsed = lEnd.QuadPart - lBegin.QuadPart;
		result = elapsed / static_cast<double>(frequency.QuadPart);
	} else {
		endBigBit = clock();
		endLittleBit = 0;
		double elapsed = endBigBit - bigBit;
		result = elapsed / CLOCKS_PER_SEC;
	}
	if (reset) {
		bigBit = endBigBit;
		littleBit = endLittleBit;
	}
	return result;
}
#endif	// WIN32

// ====================================================================
// Dynamic library handling
// ====================================================================

class DynamicLibraryImpl : public DynamicLibrary {
protected:
	lt_dlhandle m;
public:
	DynamicLibraryImpl(const char *modulePath) {
		lt_dlinit();
		m = lt_dlopen(modulePath);
	}

	virtual ~DynamicLibraryImpl() {
		if (m != NULL)
			lt_dlclose(m);
		lt_dlexit();
	}

	// Use g_module_symbol to get a pointer to the relevant function.
	virtual Function FindFunction(const char *name) {
		if (m != NULL) {
			return lt_dlsym(m, name);
		} else
			return NULL;
	}

	virtual bool IsValid() {
		return m != NULL;
	}
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath) {
	return static_cast<DynamicLibrary *>( new DynamicLibraryImpl(modulePath) );
}

// ====================================================================
// Platform
// ====================================================================

ColourDesired Platform::Chrome() {
	return ColourDesired(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight() {
	return ColourDesired(0xff, 0xff, 0xff);
}

const char *Platform::DefaultFont() {
	static FXString fontName;
	fontName = FXApp::instance()->getNormalFont()->getName();
	return fontName.text();
}

int Platform::DefaultFontSize() {
	// Warning: FOX gives the font size in deci-point
	return FXApp::instance()->getNormalFont()->getSize() / 10;
}

unsigned int Platform::DoubleClickTime() {
	return 500; 	// Half a second
}

bool Platform::MouseButtonBounce() {
	return true; // <FIXME/> same as gtk?
}

void Platform::DebugDisplay(const char *s) {
	printf("%s", s);
}

bool Platform::IsKeyDown(int) {
	// TODO: discover state of keys in GTK+/X
	return false;
}

/* These methods are now implemented in ScintillaFOX.cxx
long Platform::SendScintilla(WindowID w, unsigned int msg,
														 unsigned long wParam, long lParam) {
	return static_cast<FXScintilla *>(w)->sendMessage(msg, wParam, lParam);
}
long Platform::SendScintillaPointer(WindowID w, unsigned int msg,
																		unsigned long wParam, void *lParam) {
	return static_cast<FXScintilla *>(w)->
		sendMessage(msg, wParam, reinterpret_cast<sptr_t>(lParam));
}
*/

bool Platform::IsDBCSLeadByte(int /*codePage*/, char /*ch*/) {
	return false;
}

int Platform::DBCSCharLength(int /*codePage*/, const char *s) {
	int bytes = mblen(s, MB_CUR_MAX);
	if (bytes >= 1)
		return bytes;
	else
		return 1;
}

int Platform::DBCSCharMaxLength() {
	return MB_CUR_MAX;
}

// These are utility functions not really tied to a platform

int Platform::Minimum(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

int Platform::Maximum(int a, int b) {
	if (a > b)
		return a;
	else
		return b;
}

//#define TRACE

#ifdef TRACE
void Platform::DebugPrintf(const char *format, ...) {
	char buffer[2000];
	va_list pArguments;
	va_start(pArguments, format);
	vsprintf(buffer, format, pArguments);
	va_end(pArguments);
	Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...) {
}
#endif

// Not supported for GTK+
static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
	bool ret = assertionPopUps;
	assertionPopUps = assertionPopUps_;
	return ret;
}

void Platform::Assert(const char *c, const char *file, int line) {
	char buffer[2000];
	sprintf(buffer, "Assertion [%s] failed at %s %d", c, file, line);
	strcat(buffer, "\r\n");
	Platform::DebugDisplay(buffer);
	abort();
}

int Platform::Clamp(int val, int minVal, int maxVal) {
	if (val > maxVal)
		val = maxVal;
	if (val < minVal)
		val = minVal;
	return val;
}
