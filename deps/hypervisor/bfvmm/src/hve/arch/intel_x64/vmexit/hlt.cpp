//
// Copyright (C) 2019 Assured Information Security, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <hve/arch/intel_x64/vcpu.h>

namespace bfvmm::intel_x64
{

hlt_handler::hlt_handler(gsl::not_null<vcpu *> vcpu) :
    m_vcpu{vcpu}
{
    vcpu->add_handler(
        ::intel_x64::vmcs::exit_reason::basic_exit_reason::hlt,
        {&hlt_handler::handle, this}
    );
}

void hlt_handler::add_handler(const handler_delegate_t &d)
{
    m_handlers.push_front(d);
}

void hlt_handler::enable_exiting()
{
    using namespace ::intel_x64::vmcs;
    primary_processor_based_vm_execution_controls::hlt_exiting::enable();
}

void hlt_handler::disable_exiting()
{
    using namespace ::intel_x64::vmcs;
    primary_processor_based_vm_execution_controls::hlt_exiting::disable();
}

bool hlt_handler::handle(vcpu *vcpu)
{
    struct info_t info = {
        .ignore_advance = false
    };

    for (const auto &d : m_handlers) {
        if (d(vcpu, info)) {
            if (!info.ignore_advance) {
                vcpu->advance();
            }
            return true;
        }
    }

    throw std::runtime_error(
        "hlt_handler::handle: unhandled hlt exit"
    );
}

}