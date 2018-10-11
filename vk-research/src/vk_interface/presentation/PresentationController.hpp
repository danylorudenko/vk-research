#pragma once

#include "../../class_features/NonCopyable.hpp"

namespace VKW
{

struct PresentationControllerDesc
{

};

class PresentationController
    : public NonCopyable
{
public:
    PresentationController();
    PresentationController(PresentationControllerDesc const& desc);

    PresentationController(PresentationController&& rhs);
    PresentationController& operator=(PresentationController&& rhs);

    ~PresentationController();

private:

};

}