#pragma once

#include "base/stream.h"

namespace grabbed
{
    // note that "kameoxmlblock" data is a packed sub-resource
    
    namespace kameo
    {
        class XmlBlock
        {
        public:
            XmlBlock();

            bool Read(base::stream& stream);

        protected:
            //
        };
    }
}
