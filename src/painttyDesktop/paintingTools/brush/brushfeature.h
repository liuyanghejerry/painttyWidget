#ifndef BRUSHFEATURE_H
#define BRUSHFEATURE_H

#include <bitset>

class BrushFeature
{
public:
    enum FEATURE {
        WIDTH = 0,
        COLOR,
        HARDNESS,
        THICKNESS,
        WATER,
        EXTEND,
        MIXIN,
        MASK
    };

    static constexpr int FEATURE_COUNT = MASK+1;

    typedef std::bitset<FEATURE_COUNT> FeatureBits;

    BrushFeature(const FeatureBits& features = FeatureBits())
    {
        features_ = features;
    }

    bool support(FEATURE f)
    {
        return features_.test(f);
    }

protected:
    FeatureBits features_;
};

#endif // BRUSHFEATURE_H
