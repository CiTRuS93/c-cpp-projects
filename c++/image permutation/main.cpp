#include <memory>

#include <unordered_map>
#include <string>
#include <iostream>
#include <cassert>

#define assertmsg(exp, msg) assert(((void)msg, exp))
struct Uchar3Pixel {
    unsigned char x;
    unsigned char y;
    unsigned char z;
};

struct Frame {
    std::uint32_t m_height;
    std::uint32_t m_width;
    std::uint32_t m_pitch;
    Uchar3Pixel *m_data;
};

using ParametersContainer = std::unordered_map<std::string, std::string>;


class ProcessingModuleInterface {
public:
    virtual ~ProcessingModuleInterface() = default;

    virtual bool assignInputFrame(const Frame &frameIn) = 0;

    virtual bool assignOutputFrame(Frame &frameOut) = 0;

    virtual bool run(const ParametersContainer &params) = 0;
};


class ProcessingModule : public ProcessingModuleInterface {

public:

    /**
     *
     * @return new ProcessingModule
     */
    static std::unique_ptr<ProcessingModuleInterface> getInstance() {

        return std::make_unique<ProcessingModule>();


    };

    /*********************/

    bool assignInputFrame(const Frame &frameIn) override {
        if (ProcessingModule::checkFrameValidity(frameIn)) {
            this->in = frameIn;
            return true;
        } else {
            return false;
        }
    };

    bool assignOutputFrame(Frame &frameOut) override {
        if (ProcessingModule::checkFrameValidity(frameOut) && this->checkOutputFrame(frameOut)) {

            this->out = frameOut;
            return true;
        } else {
            return false;
        }
    };
    /**
     * apply permutation on in frame and save the results in out frame
     * @param params ParametersContainer contains the permutation
     * @return
     */
    bool run(const ParametersContainer &params) override {
        if(!(ProcessingModule::checkFrameValidity(this->in) && ProcessingModule::checkFrameValidity(this->out))){
            return false;
        }
        const static std::string PERMUTE_OPTION = "xyz";

        std::string permutation;
        try{
        permutation = params.at("OutPixelPattern");
        }
        catch (const std::out_of_range&){
            return false;
        }
        if (permutation.length() != 3) {
            return false;
        }
        for (char const c: permutation) {
            if (PERMUTE_OPTION.find(c, 0) == std::string::npos) {
                return false;
            }

        }


        for (auto y = int(); y < static_cast<int>(this->in.m_height); ++y) {
            for (auto x = int(); x < static_cast<int>(this->in.m_width); ++x) {
                try {
                    applyPermutationOnPixel(permutation, this->in.m_data[x + y *
                                                                             in.m_pitch], &this->out.m_data[x + y *
                                                                                                                in.m_pitch]);
                }
                catch (...) {
                    return false;
                }
            }
        }
        return true;
    };
    // Your code comes here.
    // You can also create additional source & header files as you like.

    /*********************/
private:

    Frame in = {};
    Frame out = {};

    bool checkOutputFrame(Frame frame) const {
        return this->in.m_height == frame.m_height && this->in.m_width == frame.m_width &&
               this->in.m_pitch == frame.m_pitch;
    }

    static bool checkFrameValidity(Frame frame) {
        if (frame.m_pitch == 0 || frame.m_width == 0 || frame.m_height == 0 || frame.m_data == nullptr) {
            return false;
        }
        return true;
    }

    static bool applyPermutationOnPixel(const std::string &permutation, Uchar3Pixel fromPixel, Uchar3Pixel *toPixel) {


        toPixel->x = ProcessingModule::uchar3PixelCharIndex2Value(permutation[0], fromPixel);
        toPixel->y = ProcessingModule::uchar3PixelCharIndex2Value(permutation[1], fromPixel);
        toPixel->z = ProcessingModule::uchar3PixelCharIndex2Value(permutation[2], fromPixel);
        return true;
    }

    static int uchar3PixelCharIndex2Value(unsigned char index, Uchar3Pixel fromPixel) {
        switch (index) {
            case 'x':
                return fromPixel.x;
            case 'y':
                return fromPixel.y;
            case 'z':
                return fromPixel.z;
            default:
                return -1;
        }

    }

};

/////
///// Just to make it compile, remove it after actually implementing this routine

// Just to make it compile, remove it after actually implementing this routine
///// Just to make it compile, remove it after actually implementing this routine
/////

int main(int argc, char *argv[]) {

    // Image properties.

    constexpr std::uint32_t IMAGE_HEIGHT = 600U;
    constexpr std::uint32_t IMAGE_WIDTH = 800U;
    constexpr std::uint32_t IMAGE_PITCH = 960U;

    // Initialize frames.
    auto inData = std::make_unique<Uchar3Pixel[]>(IMAGE_PITCH * IMAGE_HEIGHT);
    auto outData1 = std::make_unique<Uchar3Pixel[]>(IMAGE_PITCH * IMAGE_HEIGHT);
    auto outData2 = std::make_unique<Uchar3Pixel[]>(IMAGE_PITCH * IMAGE_HEIGHT);

    Frame inFrame = {
            IMAGE_HEIGHT,
            IMAGE_WIDTH,
            IMAGE_PITCH,
            inData.get()
    };

    Frame outFrame1 = {
            IMAGE_HEIGHT,
            IMAGE_WIDTH,
            IMAGE_PITCH,
            outData1.get()
    };

    Frame outFrame2 = {
            IMAGE_HEIGHT,
            IMAGE_WIDTH,
            IMAGE_PITCH,
            outData2.get()
    };

    for (auto y = int(); y < static_cast<int>(IMAGE_HEIGHT); ++y) {
        for (auto x = int(); x < static_cast<int>(IMAGE_WIDTH); ++x) {
            inFrame.m_data[x + y * IMAGE_PITCH].x = static_cast<unsigned char>(x % 256);
            inFrame.m_data[x + y * IMAGE_PITCH].y = static_cast<unsigned char>(y % 256);
            inFrame.m_data[x + y * IMAGE_PITCH].z = static_cast<unsigned char>((y + x) % 256);
        }
    }

    ParametersContainer parameters;

    auto moduleUnderTest = ProcessingModule::getInstance();



    std::cout<< "testing zxx permutation\n";
    assert(moduleUnderTest->assignInputFrame(inFrame));
    assert(moduleUnderTest->assignOutputFrame(outFrame1));
    parameters["OutPixelPattern"] = "zxx"; // Transformation of each pixel:  [x, y, z] => [z, x, x].
    assert(moduleUnderTest->run(parameters));
    for (auto y = int(); y < static_cast<int>(IMAGE_HEIGHT); ++y) {
        for (auto x = int(); x < static_cast<int>(IMAGE_WIDTH); ++x) {
            assert(inFrame.m_data[x + y * IMAGE_PITCH].z == outFrame1.m_data[x + y * IMAGE_PITCH].x);
            assert(inFrame.m_data[x + y * IMAGE_PITCH].x == outFrame1.m_data[x + y * IMAGE_PITCH].y);
            assert(inFrame.m_data[x + y * IMAGE_PITCH].x == outFrame1.m_data[x + y * IMAGE_PITCH].z);
        }
    }

    std::cout<< "testing zyx permutation\n";

    assert(moduleUnderTest->assignOutputFrame(outFrame2));



    parameters["OutPixelPattern"] = "zyx"; // Transformation of each pixel:  [x, y, z] => [z, y, x].

    assert(moduleUnderTest->run(parameters));
    for (auto y = int(); y < static_cast<int>(IMAGE_HEIGHT); ++y) {
        for (auto x = int(); x < static_cast<int>(IMAGE_WIDTH); ++x) {
            assert(inFrame.m_data[x + y * IMAGE_PITCH].z == outFrame2.m_data[x + y * IMAGE_PITCH].x);
            assert(inFrame.m_data[x + y * IMAGE_PITCH].y == outFrame2.m_data[x + y * IMAGE_PITCH].y);
            assert(inFrame.m_data[x + y * IMAGE_PITCH].x == outFrame2.m_data[x + y * IMAGE_PITCH].z);
        }
    }

    std::cout<< "testing bad permutation input\n";
    parameters["OutPixelPattern"] = "zyw"; // Fail due unknown character
    assertmsg(!moduleUnderTest->run(parameters), "Invalid permutation character");

    parameters["OutPixelPattern"] = "xyzx"; // Fail due to long string
    assertmsg(!moduleUnderTest->run(parameters), "Invalid permutation string");
    ParametersContainer emptyParameters;
    assert(!moduleUnderTest->run(emptyParameters));


    std::cout<< "testing without input frame\n";

    moduleUnderTest = ProcessingModule::getInstance();
    assertmsg(!moduleUnderTest->run(parameters), "No input frame");
    assert(moduleUnderTest->assignInputFrame(inFrame));

    std::cout<< "testing without output frame\n";
    assertmsg(!moduleUnderTest->run(parameters), "No output frame");

    std::cout<< "testing bad frames frame\n";
    Frame emptyFrame = {};
    assert(!moduleUnderTest->assignInputFrame(emptyFrame));
    assert(moduleUnderTest->assignInputFrame(inFrame));

    Frame badSizeOutFrame2 = {
            IMAGE_HEIGHT/2,
            IMAGE_WIDTH/2,
            IMAGE_PITCH/2,
            outData2.get()
    };
    assert(!moduleUnderTest->assignOutputFrame(badSizeOutFrame2));
    assert(!moduleUnderTest->assignOutputFrame(emptyFrame));
    std::cout << "done\n";
    return 0;
}