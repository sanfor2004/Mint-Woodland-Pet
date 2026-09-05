// SPDX-FileCopyrightText: 2026 sanfor2004
// SPDX-License-Identifier: MIT

#include <windows.h>      // Core Win32 window, monitor, timer, and message APIs.
#include <windowsx.h>     // Handy macros such as GET_X_LPARAM for mouse messages.
#include <gdiplus.h>      // GDI+ drawing classes: Bitmap, Graphics, Font, and Color.
#include <sapi.h>         // Windows Speech API used for optional voice output.
#include <mmsystem.h>     // PlaySound for the Mint Woodland Pet's synthesized glass-tap sound.
#include <filesystem>     // Safe paths beside the executable.
#include <fstream>        // Reading conversations.json from disk.
#include <sstream>        // Loading an entire text file into one string.
#include <random>         // Randomly choosing a line from each dialogue node.
#include <cmath>          // sin and exp for movement and sound synthesis.
#include <algorithm>      // std::min and std::max for safe limits.
#include <cstdint>        // Fixed-width integers such as int16_t.
#include <memory>         // std::unique_ptr owns each loaded sprite.
#include <map>            // Maps mood names to their sprite bitmaps.
#include "json.h"         // The project's small local JSON parser.
using namespace Gdiplus;  // Lets us write Bitmap instead of Gdiplus::Bitmap.
constexpr int W = 675, H = 435; // Transparent backing-window dimensions in pixels.
// Keep the reference composition as the largest option while using a calmer,
// smaller default for daily desktop use.
constexpr int PET_CENTER_X = 455, PET_IMAGE_BOTTOM = 391; // Anchor used by every pet-size preset.
constexpr int PET_SIZES[]{230,266,300,333}; // Available image sizes from smallest to reference size.
constexpr int PET_SIZE_COUNT = int(sizeof(PET_SIZES)/sizeof(PET_SIZES[0])); // Number of choices above.
constexpr int PET_SOURCE_LEFT = 9, PET_SOURCE_TOP = 15, PET_SOURCE_RIGHT = 83, PET_SOURCE_BOTTOM = 92; // Union of sprite pixels.
constexpr int QUESTION_X = 53, QUESTION_Y = 147, QUESTION_W = 235, QUESTION_H = 42; // Speech-bubble rectangle.
constexpr int ANSWER_RIGHT = 288, ANSWER_TOP = 208, ANSWER_GAP = 18, ANSWER_H = 37; // Stacked answer-button layout.
constexpr int SETTINGS_X = 570, SETTINGS_Y = 210, SETTINGS_SIZE = 28; // Settings-icon rectangle.
constexpr int MENU_X = 330, MENU_Y = 55, MENU_W = 270, MENU_ITEM_H = 34, MENU_ITEMS = 8; // Popup-menu layout.
constexpr ULONGLONG DEMO_INTERVAL = 8000; // Eight seconds between demo moods.
constexpr ULONGLONG REST_MESSAGE_TIME = 12000; // Hide a passive rest message after twelve seconds.
constexpr DWORD IDLE_SLEEP_TIME = 5 * 60 * 1000; // Sleep after five minutes without input.
constexpr ULONGLONG CHECK_IN_INTERVAL = 15 * 60 * 1000; // Wait fifteen minutes before an optional check-in.
constexpr ULONGLONG WATER_INTERVAL = 45 * 60 * 1000; // Wait forty-five minutes before a water prompt.
HWND windowHandle; // The one native window that hosts the transparent companion.
ISpVoice* voice = nullptr; // Optional Windows voice object; null means unavailable.
Json script; // Parsed contents of conversations.json.
std::mt19937 rng{std::random_device{}()}; // Random-number generator for dialogue-line variation.
std::string node = "greeting", mood = "happy"; // Current dialogue node and animation mood IDs.
std::wstring message = L"HI, how you doing?"; // Wide text currently shown in the question bubble.
bool demo = false, quiet = false, soundOn = false, menuOpen = false, dragging = false, conversationVisible = true; // UI state flags.
POINT grab{}; // Screen coordinate where the current drag began.
ULONGLONG started, stateAt, lastPrompt, lastWater, busyUntil = 0; // Time markers for behavior decisions.
POINT previousCursor{}; // Cursor position from the previous timer tick.
int demoIndex = 0, baseX, baseY, dragBaseX, dragBaseY, petSizeIndex = 1; // Positions and selected 266 px preset.
float phase = 0; // Slowly changing value that drives gentle idle wandering.
const std::vector<std::string> moods{"wave","idle","look","happy","sad","angry","sleep","eat","drink","thirsty","thunder"}; // Valid sprite folders.
std::map<std::string,std::unique_ptr<Bitmap>> sprites; // Loaded strip bitmap for each mood.
int previewFrame = -1; // Test renderer override; -1 means animate normally.
const std::vector<std::wstring> moodLines{L"Hi! I\'m Mint.",L"I\'ll keep you company.",L"How are you feeling?",L"That tickles!",L"A quiet hug would help.",L"Just a little grumpy!",L"Zzz... a little nap.",L"Berry break!",L"Ahh, a refreshing sip.",L"I\'m thirsty. Water break?",L"A little woodland magic!"};
int petSize() { return PET_SIZES[petSizeIndex]; } // Read the active image-size preset.
int petX() { return PET_CENTER_X-petSize()/2; } // Centre the pet horizontally at every size.
int petY() { return PET_IMAGE_BOTTOM-petSize(); } // Keep the image bottom aligned at every size.
RECT petHitBounds() {
    // All 44 frames fit inside this union of their non-transparent pixels.
    // A small pad keeps edge clicks forgiving without capturing empty canvas.
    const int size=petSize(), x=petX(), y=petY(), padding=10; // Cache scale, origin, and forgiving click margin.
    return {x+PET_SOURCE_LEFT*size/96-padding,y+PET_SOURCE_TOP*size/96-padding, // Left/top click edge after scaling.
            x+(PET_SOURCE_RIGHT*size+95)/96+padding,y+(PET_SOURCE_BOTTOM*size+95)/96+padding}; // Right/bottom edge, rounded upward.
}
std::wstring wide(const std::string& s) {
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), int(s.size()), nullptr, 0); // Ask Windows for output length.
    if (!n && !s.empty()) throw std::runtime_error("Invalid UTF-8"); // Reject malformed dialogue text.
    std::wstring out(n, 0); // Allocate exactly enough UTF-16 characters.
    MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), out.data(), n); // Perform UTF-8 to UTF-16 conversion.
    return out; // Return text suitable for Win32 and GDI+ APIs.
}
void say(const std::wstring& s) {
    if (voice && soundOn && !quiet) voice->Speak(s.c_str(), SPF_ASYNC | SPF_PURGEBEFORESPEAK, nullptr); // Speak without blocking drawing.
}
void glassTap() {
    static std::vector<char> wave; // Keep the generated WAV data after its first use.
    if(wave.empty()) {
        constexpr int samples=8820, rate=22050; // Four-tenths of a second at 22.05 kHz.
        wave.resize(44+samples*2); // Reserve a 44-byte WAV header plus 16-bit samples.
        auto number=[&](int offset,uint32_t v,int bytes) { for(int i=0;i<bytes;++i) wave[offset+i]=char((v>>(8*i))&255); }; // Write little-endian header fields.
        memcpy(wave.data(),"RIFF",4); number(4,36+samples*2,4); memcpy(wave.data()+8,"WAVEfmt ",8); // Write RIFF and format markers.
        number(16,16,4); number(20,1,2); number(22,1,2); number(24,rate,4); number(28,rate*2,4); // Describe mono PCM audio.
        number(32,2,2); number(34,16,2); memcpy(wave.data()+36,"data",4); number(40,samples*2,4); // Describe 16-bit sample data.
        for(int i=0;i<samples;++i) { double t=double(i)/rate; auto sample=int16_t(6500*std::exp(-20*t)*(std::sin(11309.7*t)+.35*std::sin(19000*t))); number(44+i*2,uint16_t(sample),2); } // Synthesize a fading glass-like tone.
    }
    if(soundOn && !quiet) PlaySoundA(wave.data(),nullptr,SND_MEMORY|SND_ASYNC|SND_NODEFAULT); // Play the in-memory sound only when allowed.
}
void state(const std::string& m, const std::wstring& s) { mood = m; message = s; stateAt = GetTickCount64(); if(m=="thirsty") glassTap(); say(s); } // Change mood, text, timing, and optional sound together.
const Json& current() { return script.at("nodes").at(node); } // Look up the active dialogue node.
void region(); // Declare this first because showConversation calls it below.
void showConversation(bool visible) {
    conversationVisible=visible; // Remember whether bubble and buttons should be painted.
    if(windowHandle) region(); // Rebuild mouse-clickable areas to match visibility.
}
void visit(const std::string& id) {
    node = id; // Move to the requested conversation node.
    const auto& n = current(); // Read the JSON object for that node.
    const auto& lines = n.at("lines").array; // Get its alternative message lines.
    state(n.at("mood").text, wide(lines[rng() % lines.size()].text)); // Pick one line and matching mood.
    showConversation(true); // A visited node always opens the conversation UI.
    lastPrompt = GetTickCount64(); // Restart the timer for automatic check-ins.
}
RECT contentBounds() {
    // Window placement is based on visible controls and sprite pixels, not the
    // large transparent backing bitmap.
    RECT pet=petHitBounds(); // Start with the visible pet click rectangle.
    RECT content{std::min(pet.left,LONG(SETTINGS_X)),std::min(pet.top,LONG(SETTINGS_Y)),std::max(pet.right,LONG(SETTINGS_X+SETTINGS_SIZE)),std::max(pet.bottom,LONG(SETTINGS_Y+SETTINGS_SIZE))}; // Include the settings control.
    if(conversationVisible) {
        content.left=std::min(content.left,45L); content.top=std::min(content.top,LONG(QUESTION_Y)); // Expand for bubble and answers.
        content.right=std::max(content.right,LONG(ANSWER_RIGHT)); content.bottom=std::max(content.bottom,LONG(ANSWER_TOP+ANSWER_H*2+ANSWER_GAP)); // Include second answer.
    }
    if(menuOpen) {
        content.left=std::min(content.left,LONG(MENU_X)); content.top=std::min(content.top,LONG(MENU_Y)); // Expand for menu origin.
        content.right=std::max(content.right,LONG(MENU_X+MENU_W)); content.bottom=std::max(content.bottom,LONG(MENU_Y+MENU_ITEM_H*MENU_ITEMS+16)); // Expand for menu bottom.
    }
    return content; // Give callers the bounding rectangle of all visible UI.
}
void moveWindow(int x,int y) {
    // Do not clamp during a drag. This avoids jumps as the nearest monitor
    // changes; the final mouse-up applies work-area limits once.
    baseX=x; baseY=y; // Save the new backing-window origin.
    SetWindowPos(windowHandle,nullptr,baseX,baseY,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE); // Move without stealing focus.
}
void bounds(int x, int y) {
    const RECT content=contentBounds(); // Calculate what must remain visible.
    RECT r{x+content.left,y+content.top,x+content.right,y+content.bottom}; MONITORINFO mi{sizeof(mi)}; // Make a screen-space candidate rectangle.
    GetMonitorInfo(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi); // Choose the nearest monitor's usable area.
    baseX = std::max<int>(mi.rcWork.left-content.left, std::min<int>(x, mi.rcWork.right-content.right)); // Clamp horizontally using visible content.
    baseY = std::max<int>(mi.rcWork.top-content.top, std::min<int>(y, mi.rcWork.bottom-content.bottom)); // Clamp vertically above taskbar.
    SetWindowPos(windowHandle, HWND_TOPMOST, baseX, baseY, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE); // Apply final topmost location.
}
void region() {
    const RECT pet=petHitBounds(); // Reuse the tight pet target for hit testing.
    HRGN r = CreateRectRgn(pet.left,pet.top,pet.right,pet.bottom); // Start the clickable window region with the pet.
    if(conversationVisible) {
        HRGN q = CreateRoundRectRgn(QUESTION_X,QUESTION_Y,QUESTION_X+QUESTION_W,QUESTION_Y+QUESTION_H,16,16); CombineRgn(r,r,q,RGN_OR); DeleteObject(q); // Add bubble then free temp region.
        HRGN answers = CreateRectRgn(45,ANSWER_TOP,ANSWER_RIGHT,ANSWER_TOP+ANSWER_H*2+ANSWER_GAP); CombineRgn(r,r,answers,RGN_OR); DeleteObject(answers); // Add both answer buttons.
    }
    HRGN settings = CreateRoundRectRgn(SETTINGS_X,SETTINGS_Y,SETTINGS_X+SETTINGS_SIZE,SETTINGS_Y+SETTINGS_SIZE,16,16); CombineRgn(r,r,settings,RGN_OR); DeleteObject(settings); // Add settings control.
    if (menuOpen) { HRGN m = CreateRoundRectRgn(MENU_X,MENU_Y,MENU_X+MENU_W,MENU_Y+MENU_ITEM_H*MENU_ITEMS+16,16,16); CombineRgn(r,r,m,RGN_OR); DeleteObject(m); } // Add popup only when shown.
    SetWindowRgn(windowHandle,r,TRUE); // Give ownership of the final region to Windows.
}
void resizePet(int direction) {
    // The image stays anchored around the same center and baseline at every size.
    const int next=std::max(0,std::min(PET_SIZE_COUNT-1,petSizeIndex+direction)); // Keep index inside available presets.
    if(next==petSizeIndex) return; // Do nothing when already smallest or largest.
    petSizeIndex=next; // Select the requested size.
    region(); bounds(baseX,baseY); InvalidateRect(windowHandle,nullptr,FALSE); // Refresh hit area, placement, and drawing.
}
void ellipse(Graphics& g, Color c, float x,float y,float w,float h) { SolidBrush b(c); g.FillEllipse(&b,x,y,w,h); } // Fill one ellipse with a solid color.
void round(Graphics& g, Color c,float x,float y,float w,float h,float r=24) {
    // This window uses magenta as its transparent color key. Antialiasing a
    // filled edge against that key leaves a pink fringe after Windows removes
    // the background, so rounded control surfaces need hard, solid edge pixels.
    const auto smoothing = g.GetSmoothingMode(); // Remember the caller's drawing setting.
    g.SetSmoothingMode(SmoothingModeNone); // Prevent blended magenta edge pixels.
    GraphicsPath path; path.AddArc(x,y,r*2,r*2,180,90); path.AddArc(x+w-r*2,y,r*2,r*2,270,90); // Draw top-left and top-right arcs.
    path.AddArc(x+w-r*2,y+h-r*2,r*2,r*2,0,90); path.AddArc(x,y+h-r*2,r*2,r*2,90,90); path.CloseFigure(); SolidBrush b(c); g.FillPath(&b,&path); // Finish, fill, and close the rounded shape.
    g.SetSmoothingMode(smoothing); // Restore the caller's preferred smoothing.
}
void text(Graphics& g,const std::wstring& s,float x,float y,float w,float h,int size=15, Color color=Color(40,44,52)) {
    FontFamily family(L"Segoe UI"); // Select the standard menu font.
    Font font(&family,float(size),FontStyleRegular,UnitPixel); // Create a font at the requested pixel size.
    SolidBrush b(color); // Make a brush for the glyph color.
    StringFormat format; format.SetLineAlignment(StringAlignmentCenter); // Vertically centre text in its rectangle.
    g.DrawString(s.c_str(),-1,&font,RectF(x,y,w,h),&format,&b); // Render the wide string.
}
void monoText(Graphics& g,const std::wstring& s,float x,float y,float w,float h,float size,Color color,bool centered=false,bool bold=false) {
    FontFamily family(L"Consolas"); // Use the reference UI's monospace appearance.
    Font font(&family,size,bold?FontStyleBold:FontStyleRegular,UnitPixel); // Choose normal or bold face.
    SolidBrush b(color); // Use the requested text color.
    StringFormat format; format.SetLineAlignment(StringAlignmentCenter); format.SetAlignment(centered?StringAlignmentCenter:StringAlignmentNear); // Control vertical and horizontal alignment.
    format.SetTrimming(StringTrimmingNone); format.SetFormatFlags(StringFormatFlagsNoWrap); // Do not wrap or replace text with ellipsis.
    g.DrawString(s.c_str(),-1,&font,RectF(x,y,w,h),&format,&b); // Draw the actual label.
}
void questionText(Graphics& g,const std::wstring& s,float x,float y,float w,float h) {
    FontFamily family(L"Consolas"); // Keep the question in the reference UI's pixel-like monospace face.
    SolidBrush brush(Color(104,42,70)); // Use the same plum question color at every fitted size.
    StringFormat format; format.SetAlignment(StringAlignmentNear); format.SetLineAlignment(StringAlignmentCenter); // Left-align text and center one or two lines vertically.
    format.SetTrimming(StringTrimmingEllipsisCharacter); format.SetFormatFlags(StringFormatFlagsLineLimit); // Wrap at words, limit drawing to the bubble, and keep a safe fallback.
    for(float size=20.5f;size>=12.0f;size-=0.5f) { // Try the reference size first, then shrink only as much as the message needs.
        Font font(&family,size,FontStyleRegular,UnitPixel); // Build this candidate font.
        RectF measured; int charactersFitted=0,linesFilled=0; // Receive how much text fits inside the bubble's padded rectangle.
        g.MeasureString(s.c_str(),-1,&font,RectF(x,y,w,h),&format,&measured,&charactersFitted,&linesFilled); // Measure with the same wrapping rules used for drawing.
        if(charactersFitted>=int(s.size())&&linesFilled<=2) { g.DrawString(s.c_str(),-1,&font,RectF(x,y,w,h),&format,&brush); return; } // Draw the largest complete one- or two-line fit.
    }
    Font fallback(&family,12.0f,FontStyleRegular,UnitPixel); g.DrawString(s.c_str(),-1,&fallback,RectF(x,y,w,h),&format,&brush); // Extremely long custom text stays inside with ellipsis.
}
int answerWidth(const std::wstring& label) { return (std::max)(93,(std::min)(220,13+int(label.size())*10)); } // Estimate a compact safe width from character count.
void drawSettingsIcon(Graphics& g) {
    round(g,Color(6,66,69),SETTINGS_X,SETTINGS_Y,SETTINGS_SIZE,SETTINGS_SIZE,8); // Draw dark-teal rounded button.
    Pen rail(Color(205,236,231),1.5f); // Choose the pale line color for sliders.
    const float xs[3]{577.0f,584.0f,591.0f}; const float knobs[3]{221.0f,228.0f,218.0f}; // Three slider rails and knob heights.
    for(int i=0;i<3;++i) {
        g.DrawLine(&rail,xs[i],215.0f,xs[i],233.0f); // Draw one vertical slider rail.
        SolidBrush fill(Color(6,66,69)); g.FillEllipse(&fill,xs[i]-2.0f,knobs[i]-2.0f,4.0f,4.0f); // Paint its knob interior.
        g.DrawEllipse(&rail,xs[i]-2.0f,knobs[i]-2.0f,4.0f,4.0f); // Outline the knob.
    }
}
void drawConversation(Graphics& g,const std::wstring& line,const Json& conversation) {
    round(g,Color(255,255,255),QUESTION_X,QUESTION_Y,QUESTION_W,QUESTION_H,8); // Draw white question bubble.
    questionText(g,line,65,149,211,38); // Wrap and fit long questions inside the bubble while preserving the reference size for short text.
    const auto& options=conversation.at("answers").array; // Read the node's two JSON answer choices.
    for(size_t i=0;i<options.size()&&i<2;++i) {
        const auto label=wide(options[i].at("label").text); const int width=answerWidth(label); // Convert label and size its button.
        const float y=float(ANSWER_TOP+int(i)*(ANSWER_H+ANSWER_GAP)); const float x=float(ANSWER_RIGHT-width); // Stack it and align its right edge.
        round(g,Color(6,66,69),x,y,float(width),ANSWER_H,8); // Draw dark-teal answer surface.
        monoText(g,label,x+8,y,float(width-16),ANSWER_H,17,Color(255,255,255),true); // Centre white answer text.
    }
    drawSettingsIcon(g); // Draw settings last so it remains visible.
}
int moodDuration(const std::string& id) { return id=="sleep"?450:id=="thunder"?160:240; } // Give sleep slower and thunder faster animation frames.
void drawPet(Graphics& g) {
    auto found=sprites.find(mood); // Find the bitmap strip matching current mood.
    if(found==sprites.end()) return; // Do not draw if a required bitmap is missing.
    const auto saved=g.Save(); // Preserve Graphics settings for the caller.
    const auto elapsed=GetTickCount64()-stateAt; // Measure current state's age.
    const int frame=previewFrame>=0?previewFrame:quiet?0:int(elapsed/moodDuration(mood))%4; // Pick a four-frame loop cell.
    int dash=mood=="thunder"&&!quiet?int(std::sin(std::min(1.0,elapsed/1500.0)*3.14159)*12):0; // Create the playful thunder hop.
    g.SetInterpolationMode(InterpolationModeNearestNeighbor); // Keep pixel art sharp when scaled.
    g.SetPixelOffsetMode(PixelOffsetModeHalf); // Align scaled pixel sampling cleanly.
    g.DrawImage(found->second.get(),Rect(petX()+dash,petY()-dash,petSize(),petSize()),frame*96,0,96,96,UnitPixel); // Draw only selected 96 px strip cell.
    g.Restore(saved); // Undo pixel-art-specific Graphics settings.
}
void paint(HDC hdc) {
    Bitmap buffer(W,H,PixelFormat32bppARGB); // Make an off-screen image to avoid flickering.
    Graphics g(&buffer); // Draw into that off-screen image.
    g.Clear(Color(255,0,255)); // Fill transparent-key pixels with magenta.
    g.SetSmoothingMode(SmoothingModeAntiAlias); // Smooth text and settings-icon lines.
    const auto elapsed = GetTickCount64()-stateAt; // Find delay since state changed.
    if(conversationVisible && elapsed>250) drawConversation(g,message,current()); // Reveal controls after short transition delay.
    drawPet(g); // Draw the Mint Woodland Pet above the conversation surface.
    if (menuOpen) {
        round(g,Color(255,255,255),MENU_X,MENU_Y,MENU_W,MENU_ITEM_H*MENU_ITEMS+16,8); // Draw white settings popup.
        std::vector<std::wstring> labels{demo?L"✓ Demo loop — stop":L"Demo loop — start",quiet?L"✓ Quiet mode":L"Quiet mode",soundOn?L"✓ Voice on":L"Voice off",L"Make Mint smaller",L"Make Mint larger",L"Move to next screen",L"Ask a question",L"Close Mint"}; // Build labels from state.
        for (int i=0;i<MENU_ITEMS;++i) text(g,labels[i],MENU_X+16,float(MENU_Y+8+i*MENU_ITEM_H),MENU_W-32,MENU_ITEM_H,14,Color(30,54,55)); // Draw every menu row.
    }
    Graphics out(hdc); // Wrap the paint event's device context.
    out.DrawImage(&buffer,0,0); // Copy completed frame to the native window.
}
BOOL CALLBACK collect(HMONITOR monitor,HDC,LPRECT,LPARAM arg) { reinterpret_cast<std::vector<HMONITOR>*>(arg)->push_back(monitor); return TRUE; } // Collect monitor handles during enumeration.
LRESULT CALLBACK procedure(HWND h,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) { // Windows calls this once for every event sent to the pet's window.
    case WM_ERASEBKGND: return 1; // Skip default erase because paint draws the whole frame.
    case WM_PAINT: { PAINTSTRUCT ps; HDC dc=BeginPaint(h,&ps); paint(dc); EndPaint(h,&ps); return 0; } // Render one frame during paint event.
    case WM_TIMER: {
        phase += .035f; ULONGLONG now=GetTickCount64(); // Advance idle-wander phase and read current clock.
        LASTINPUTINFO input{sizeof(input)}; GetLastInputInfo(&input); DWORD idle=GetTickCount()-input.dwTime; // Measure time since user input.
        POINT cursor; GetCursorPos(&cursor); // Read global cursor location.
        if(abs(cursor.x-previousCursor.x)+abs(cursor.y-previousCursor.y)>20) busyUntil=now+2000; // Pause wandering after active pointer movement.
        previousCursor=cursor; // Save point for next timer tick.
        if (!menuOpen && !dragging) {
            if (demo && now-stateAt>DEMO_INTERVAL) { demoIndex=(demoIndex+1)%int(moods.size()); state(moods[demoIndex],moodLines[demoIndex]); showConversation(true); } // Advance optional showcase loop.
            else if (!demo && idle>IDLE_SLEEP_TIME && mood!="sleep") { state("sleep",L"Zzz... I'll be here."); showConversation(false); } // Sleep quietly after inactivity.
            else if (!demo && idle<1000 && mood=="sleep") { state("look",L"Welcome back!"); showConversation(false); } // Wake when user returns.
            else if (!demo && !quiet && node=="rest" && idle<IDLE_SLEEP_TIME && now-lastWater>WATER_INTERVAL) { lastWater=now; visit("water"); } // Give an occasional water reminder.
            else if (!demo && !quiet && node=="rest" && idle<IDLE_SLEEP_TIME && now-lastPrompt>CHECK_IN_INTERVAL) visit("greeting"); // Offer rare check-in only at rest.
            else if (!demo && node=="rest" && conversationVisible && now-stateAt>REST_MESSAGE_TIME) showConversation(false); // Collapse a passive message.
            else if (!demo && node=="rest" && (mood=="thunder" || mood=="happy" || mood=="wave" || mood=="eat" || mood=="drink") && now-stateAt>3500) state("idle",L"I'll stay nearby."); // Return short reactions to idle.
            if (!quiet && mood!="sleep" && !demo && now>busyUntil && now-stateAt>3500 && node=="rest") {
                SetWindowPos(h,nullptr,baseX+int(std::sin(phase/8)*8),baseY,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE); // Apply tiny idle sway.
            }
        }
        InvalidateRect(h,nullptr,FALSE); return 0; // Ask Windows to schedule the next visual update.
    }
    case WM_LBUTTONDOWN: {
        int x=GET_X_LPARAM(lp), y=GET_Y_LPARAM(lp); // Extract clicked local coordinates.
        if (x>=SETTINGS_X && x<SETTINGS_X+SETTINGS_SIZE && y>=SETTINGS_Y && y<SETTINGS_Y+SETTINGS_SIZE) { menuOpen=!menuOpen; region(); } // Toggle settings popup.
        else if(menuOpen && x>=MENU_X && x<MENU_X+MENU_W && y>=MENU_Y+8 && y<MENU_Y+8+MENU_ITEM_H*MENU_ITEMS) {
            int item=(y-(MENU_Y+8))/MENU_ITEM_H; // Turn click row into menu-item number.
            if(item==0) { demo=!demo; demoIndex=0; if(!demo) { visit("rest"); showConversation(false); } else { state("wave",moodLines[0]); showConversation(true); } } // Toggle showcase mode.
            if(item==1) { quiet=!quiet; if(quiet) { demo=false; visit("rest"); showConversation(false); PlaySound(nullptr,nullptr,0); if(voice) voice->Speak(L"",SPF_ASYNC|SPF_PURGEBEFORESPEAK,nullptr); } } // Toggle quiet and silence output.
            if(item==2) { soundOn=!soundOn; if(soundOn) say(L"Hi! Voice is on."); else if(voice) voice->Speak(L"",SPF_ASYNC|SPF_PURGEBEFORESPEAK,nullptr); } // Toggle voice feedback.
            if(item==3) resizePet(-1); // Move to smaller image preset.
            if(item==4) resizePet(1); // Move to larger image preset.
            if(item==5) { std::vector<HMONITOR> all; EnumDisplayMonitors(nullptr,nullptr,collect,reinterpret_cast<LPARAM>(&all)); auto cur=MonitorFromWindow(h,MONITOR_DEFAULTTONEAREST); size_t i=0; while(i<all.size()&&all[i]!=cur) ++i; if(!all.empty()) { MONITORINFO mi{sizeof(mi)}; GetMonitorInfo(all[(i+1)%all.size()],&mi); const RECT content=contentBounds(); bounds(mi.rcWork.right-content.right-20,mi.rcWork.bottom-content.bottom-20); } } // Move to next monitor's bottom-right work area.
            if(item==6) { demo=false; visit("greeting"); } // Open a fresh manual question.
            if(item==7) DestroyWindow(h); // Close the companion window.
            menuOpen=false; if(IsWindow(h)) region(); // Close menu and shrink click region.
        } else if(x<=ANSWER_RIGHT && ((y>=ANSWER_TOP&&y<ANSWER_TOP+ANSWER_H)||(y>=ANSWER_TOP+ANSWER_H+ANSWER_GAP&&y<ANSWER_TOP+ANSWER_H*2+ANSWER_GAP))) {
            size_t index=y>=ANSWER_TOP+ANSWER_H+ANSWER_GAP?1:0; const auto& choices=current().at("answers").array; // Select first or second JSON choice.
            if(index<choices.size()) { demo=false; visit(choices[index].at("next").text); } // Follow its next-node link.
        } else { const RECT pet=petHitBounds(); if(x>=pet.left&&x<pet.right&&y>=pet.top&&y<pet.bottom) { dragging=true; GetCursorPos(&grab); dragBaseX=baseX; dragBaseY=baseY; SetCapture(h); } } // Begin a pet click or drag.
        InvalidateRect(h,nullptr,FALSE); return 0; // Refresh after any click action.
    }
    case WM_MOUSEMOVE: if(dragging) { POINT p; GetCursorPos(&p); moveWindow(dragBaseX+p.x-grab.x,dragBaseY+p.y-grab.y); return 0; } break; // Move smoothly while pointer is held.
    case WM_LBUTTONUP: if(dragging) { POINT p; GetCursorPos(&p); int dx=p.x-grab.x,dy=p.y-grab.y; dragging=false; ReleaseCapture(); if(abs(dx)+abs(dy)<6) { demo=false; state("thunder",L"Mint! That tickles!"); showConversation(true); } else bounds(dragBaseX+dx,dragBaseY+dy); } return 0; // Pet on click; clamp only after drag release.
    case WM_CAPTURECHANGED: dragging=false; return 0; // Stop dragging if another window takes mouse capture.
    case WM_DISPLAYCHANGE: bounds(baseX,baseY); return 0; // Re-clamp after monitor setup changes.
    case WM_DPICHANGED: bounds(baseX,baseY); return 0; // Re-clamp after a mixed-DPI monitor crossing.
    case WM_KEYDOWN: if(wp==VK_ESCAPE) DestroyWindow(h); return 0; // Escape closes Mint.
    case WM_DESTROY: KillTimer(h,1); PostQuitMessage(0); return 0; // Cleanly end the app message loop.
    }
    return DefWindowProc(h,msg,wp,lp); // Let Windows handle any event we did not use.
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR arguments,int) {
    // Use physical monitor coordinates on mixed-DPI desktops when supported.
    using SetDpiContext = BOOL(WINAPI*)(HANDLE); // Describe optional newer Windows DPI function.
    const auto setDpiContext=reinterpret_cast<SetDpiContext>(GetProcAddress(GetModuleHandle(L"user32.dll"),"SetProcessDpiAwarenessContext")); // Look it up safely at runtime.
    if(setDpiContext) setDpiContext(reinterpret_cast<HANDLE>(static_cast<INT_PTR>(-4))); // Request per-monitor DPI awareness v2.
    else SetProcessDPIAware(); // Fall back for older Windows versions.
    try {
        wchar_t path[MAX_PATH]; GetModuleFileName(nullptr,path,MAX_PATH); // Find running executable's full path.
        std::ifstream file(std::filesystem::path(path).parent_path()/L"data"/L"conversations.json"); // Open adjacent dialogue file.
        if(!file) throw std::runtime_error("Cannot open data/conversations.json beside the executable"); // Stop with useful error if missing.
        std::stringstream contents; contents<<file.rdbuf(); auto source=contents.str(); script=JsonParser(source).parse(); // Read and parse JSON.
        for(const auto& pair:script.at("nodes").object) {
            const auto& n=pair.second; // Read one dialogue node.
            if(n.at("lines").array.empty() || n.at("answers").array.size()!=2) throw std::runtime_error("Every node needs lines and two answers"); // Validate required structure.
            for(const auto& line:n.at("lines").array) wide(line.text); // Verify every displayed line is valid UTF-8.
            for(const auto& a:n.at("answers").array) { wide(a.at("label").text); script.at("nodes").at(a.at("next").text); } // Verify labels and linked nodes.
        }
        current(); // Confirm the initial greeting node exists.
    } catch(const std::exception& e) { MessageBoxA(nullptr,e.what(),"Mint Woodland Pet configuration error",MB_ICONERROR); return 1; } // Show configuration errors before window creation.
    CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED); // Start COM for speech services.
    CoCreateInstance(CLSID_SpVoice,nullptr,CLSCTX_ALL,IID_ISpVoice,reinterpret_cast<void**>(&voice)); // Try to create optional Windows voice.
    ULONG_PTR token; GdiplusStartupInput gd; GdiplusStartup(&token,&gd,nullptr); // Start GDI+ drawing library.
    wchar_t binaryPath[MAX_PATH]; GetModuleFileName(nullptr,binaryPath,MAX_PATH); // Find executable again for asset paths.
    const auto assetRoot=std::filesystem::path(binaryPath).parent_path()/L"assets"/L"mint-woodland-pet"/L"moods"; // Locate shipped mood strips.
    for(const auto& id:moods) {
        auto sprite=std::make_unique<Bitmap>((assetRoot/wide(id)/L"strip.png").c_str()); // Load one 384×96 PNG strip.
        if(sprite->GetLastStatus()!=Ok || sprite->GetWidth()!=384 || sprite->GetHeight()!=96) {
            MessageBox(nullptr,(L"Missing or invalid Mint Woodland Pet sprite: "+wide(id)+L". Keep assets/mint-woodland-pet/moods beside the executable.").c_str(),L"Mint Woodland Pet asset error",MB_ICONERROR); // Explain invalid asset.
            sprites.clear(); if(voice)voice->Release();CoUninitialize();GdiplusShutdown(token);return 2; // Release libraries then fail safely.
        }
        sprites.emplace(id,std::move(sprite)); // Store bitmap under its mood name.
    }
    if(std::wstring(arguments)==L"--validate") {
        bool valid=script.at("nodes").at("greeting").at("lines").array.size()==20; // Require twenty greeting options.
        for(const auto& pair:script.at("nodes").object) valid=valid&&sprites.count(pair.second.at("mood").text)>0; // Require a sprite for every node mood.
        sprites.clear();if(voice)voice->Release();CoUninitialize();GdiplusShutdown(token);return valid?0:3; // Exit without opening UI.
    }
    if(std::wstring(arguments)==L"--render") {
        wchar_t executable[MAX_PATH]; GetModuleFileName(nullptr,executable,MAX_PATH); // Locate the build output directory.
        auto output=std::filesystem::path(executable).parent_path()/L"previews"; std::filesystem::create_directories(output); // Ensure preview folder exists.
        CLSID png; CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}",&png); // Ask GDI+ for PNG encoder.
        Bitmap sheet(1280,1020,PixelFormat32bppARGB); // Prepare an all-moods contact sheet.
        {
            Graphics g(&sheet); g.Clear(Color(235,236,229)); g.SetSmoothingMode(SmoothingModeAntiAlias); // Start drawing contact sheet.
            text(g,L"MINT WOODLAND PET / pixel-art moods",30,12,850,60,28); // Draw contact-sheet title.
            text(g,L"11 moods / generated sprites / nearest-neighbor animation",30,65,1100,34,16); // Draw subtitle.
            for(int i=0;i<int(moods.size());++i) {
                int x=20+(i%4)*315,y=116+(i/4)*295; round(g,Color(250,249,244),float(x),float(y),300,280,24); // Place one card in four-column grid.
                mood=moods[i]; stateAt=GetTickCount64()-750; phase=1; previewFrame=2; // Force consistent sample frame.
                auto saved=g.Save(); g.TranslateTransform(float(x)-340,float(y)-68); drawPet(g); g.Restore(saved); // Draw translated pet into card.
                text(g,wide(moods[i]),float(x)+20,float(y)+223,260,26,18); // Label the mood.
                text(g,moodLines[i],float(x)+20,float(y)+249,270,26,13); // Label its sample line.
            }
        }
        auto status=sheet.Save((output/L"mood-sheet.png").c_str(),&png,nullptr); // Save the contact sheet.
        for(int i=0;i<int(moods.size());++i) {
            Bitmap frame(W,H,PixelFormat32bppARGB); // Make one full UI preview per mood.
            { Graphics g(&frame); g.Clear(Color(102,102,102)); g.SetSmoothingMode(SmoothingModeAntiAlias);
              mood=moods[i]; message=moodLines[i]; stateAt=GetTickCount64()-750; // Choose sample mood and text.
              drawConversation(g,message,current()); drawPet(g); // Render UI and pet together.
            }
            frame.Save((output/(wide(moods[i])+L".png")).c_str(),&png,nullptr); // Save this mood's preview.
        }
        {
            Bitmap reference(W,H,PixelFormat32bppARGB); Graphics g(&reference); g.Clear(Color(102,102,102)); g.SetSmoothingMode(SmoothingModeAntiAlias); // Build reference comparison frame.
            const int savedSize=petSizeIndex; petSizeIndex=PET_SIZE_COUNT-1; // Temporarily select 333 px reference size.
            mood="happy"; message=L"HI, how you doing?"; previewFrame=0; stateAt=GetTickCount64(); drawConversation(g,message,current()); drawPet(g); // Draw exact reference scene.
            petSizeIndex=savedSize; // Restore user's active size preset.
            reference.Save((output/L"ui-reference-render.png").c_str(),&png,nullptr); // Save comparison image.
        }
        {
            Bitmap longQuestion(W,H,PixelFormat32bppARGB); Graphics g(&longQuestion); g.Clear(Color(102,102,102)); g.SetSmoothingMode(SmoothingModeAntiAlias); // Create a regression preview for the longest question style.
            const auto savedNode=node; node="bad"; mood="sad"; message=L"Sorry it's a rough one. What would help right now?"; previewFrame=0; stateAt=GetTickCount64(); // Select the longest real dialogue line and its answers.
            drawConversation(g,message,current()); drawPet(g); node=savedNode; // Render the fitted two-line question, then restore dialogue state.
            longQuestion.Save((output/L"long-question-render.png").c_str(),&png,nullptr); // Keep the visual regression image beside other native previews.
        }
        if(voice) voice->Release(); CoUninitialize(); // Release optional COM voice before exit.
        // Keep GDI+ alive until the local Bitmap objects have been destroyed.
        return status==Ok?0:2; // Report whether contact sheet saved successfully.
    }
    WNDCLASS wc{}; wc.hInstance=instance; wc.lpfnWndProc=procedure; wc.lpszClassName=L"MintWoodlandPetPrototype"; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); RegisterClass(&wc); // Register custom window type.
    windowHandle=CreateWindowEx(WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_TOOLWINDOW,wc.lpszClassName,L"Mint Woodland Pet",WS_POPUP,0,0,W,H,nullptr,nullptr,instance,nullptr); // Create borderless overlay.
    SetLayeredWindowAttributes(windowHandle,RGB(255,0,255),255,LWA_COLORKEY); region(); // Make magenta transparent and set click area.
    RECT work; SystemParametersInfo(SPI_GETWORKAREA,0,&work,0); const RECT initialContent=contentBounds(); // Read primary usable desktop area.
    bounds(work.right-initialContent.right-20,work.bottom-initialContent.bottom-20); // Start near bottom-right, above taskbar.
    if(std::wstring(arguments)==L"--self-test") {
        // Exercise this application's actual window handlers without sending
        // global mouse/keyboard input or disturbing another application.
        stateAt=lastPrompt=lastWater=GetTickCount64(); soundOn=false; // Reset timing and prevent spoken test output.
        auto click=[&](int x,int y){SendMessage(windowHandle,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));SendMessage(windowHandle,WM_LBUTTONUP,0,MAKELPARAM(x,y));}; // Send a synthetic UI click.
        auto petClick=[&](int x,int y){SendMessage(windowHandle,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));GetCursorPos(&grab);dragging=true;SendMessage(windowHandle,WM_LBUTTONUP,0,MAKELPARAM(x,y));}; // Send synthetic pet click without global input.
        int failure=0; auto check=[&](bool value,int code){if(!value&&!failure)failure=code;}; // Preserve first failing test number.
        visit("greeting");click(240,225);check(node=="good"&&!demo,1); // Test first greeting answer.
        visit("greeting");click(240,280);check(node=="bad"&&mood=="sad",2); // Test second greeting answer.
        visit("water");click(240,225);check(node=="hydrated"&&mood=="drink",3); // Test water branch.
        visit("mint");click(240,225);check(node=="snack"&&mood=="eat",4); // Test food branch.
        petClick(480,230);check(mood=="thunder",5); // Test pet reaction.
        const int originalSize=petSizeIndex; // Remember default size for menu checks.
        click(584,225);click(380,182);check(petSizeIndex==originalSize-1,6); // Test smaller size action.
        click(584,225);click(380,216);check(petSizeIndex==originalSize,7); // Test larger size action restores it.
        click(584,225);check(menuOpen,8);click(380,110);check(quiet&&!demo&&!menuOpen,9); // Test quiet mode.
        click(584,225);click(380,110);check(!quiet,10); // Test leaving quiet mode.
        click(584,225);click(380,80);check(demo&&demoIndex==0&&mood=="wave",11); // Test demo start.
        MONITORINFO testMonitor{sizeof(testMonitor)}; GetMonitorInfo(MonitorFromWindow(windowHandle,MONITOR_DEFAULTTONEAREST),&testMonitor); // Read monitor limits.
        bounds(testMonitor.rcWork.right,testMonitor.rcWork.bottom); const RECT testContent=contentBounds(); // Request far bottom-right placement.
        check(baseX+testContent.right==testMonitor.rcWork.right&&baseY+testContent.bottom==testMonitor.rcWork.bottom,12); // Verify visible content reaches both edges.
        DestroyWindow(windowHandle);sprites.clear();if(voice)voice->Release();CoUninitialize();GdiplusShutdown(token);return failure?10+failure:0; // Clean up and report test result.
    }
    started=stateAt=lastPrompt=lastWater=GetTickCount64(); GetCursorPos(&previousCursor); SetTimer(windowHandle,1,33,nullptr); ShowWindow(windowHandle,SW_SHOWNOACTIVATE); // Initialize behavior, run 30 FPS timer, then show Mint without focus.
    MSG msg; while(GetMessage(&msg,nullptr,0,0)>0) { TranslateMessage(&msg); DispatchMessage(&msg); } // Process Windows events until closed.
    sprites.clear(); if(voice) voice->Release(); CoUninitialize(); GdiplusShutdown(token); return 0; // Release all resources on normal exit.
}
