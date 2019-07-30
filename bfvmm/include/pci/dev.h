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

#ifndef MICROV_PCI_DEV_H
#define MICROV_PCI_DEV_H

#include <array>
#include <memory>

#include <bfgsl.h>
#include <hve/arch/intel_x64/vcpu.h>

#include "bar.h"
#include "cfg.h"

namespace microv {

struct pci_dev {
    using vcpu = intel_x64::vcpu;
    using base_vcpu = ::bfvmm::intel_x64::vcpu;
    using cfg_info = intel_x64::pci_cfg_handler::info;

    static constexpr size_t vcfg_size = 1024;

    uint32_t m_cf8{};
    uint32_t m_msi_cap{};
    uint32_t m_pcie_cap{};
    uintptr_t m_ecam_gpa{};
    uintptr_t m_ecam_hpa{};

    bool m_guest_owned{};
    struct pci_dev *m_bridge{};
    std::array<uint32_t, 4> m_cfg_reg{};
    std::unique_ptr<uint32_t[]> m_vcfg{};
    pci_bar_list m_bars{};

    void parse_bars()
    {
        pci_parse_bars(m_cf8, m_bars);
    }

    bool is_netdev() const
    {
        return pci_cfg_is_netdev(m_cfg_reg[2]);
    }

    bool is_host_bridge() const
    {
        return pci_cfg_is_host_bridge(m_cfg_reg[2]);
    }

    bool is_pci_bridge() const
    {
        return pci_cfg_is_pci_bridge(m_cfg_reg[3]);
    }

    bool is_normal() const
    {
        return pci_cfg_is_normal(m_cfg_reg[3]);
    }

    void remap_ecam();
    void parse_cap_regs();
    void init_host_vcfg();
    void add_host_handlers(vcpu *vcpu);
    void add_guest_handlers(vcpu *vcpu);

    bool host_cfg_in(base_vcpu *vcpu, cfg_info &info);
    bool host_cfg_out(base_vcpu *vcpu, cfg_info &info);
    bool guest_cfg_out(base_vcpu *vcpu, cfg_info &info);
    bool guest_host_bridge_cfg_in(base_vcpu *vcpu, cfg_info &info);
    bool guest_host_bridge_cfg_out(base_vcpu *vcpu, cfg_info &info);
    bool guest_pci_bridge_cfg_in(base_vcpu *vcpu, cfg_info &info);
    bool guest_pci_bridge_cfg_out(base_vcpu *vcpu, cfg_info &info);
    bool guest_normal_cfg_in(base_vcpu *vcpu, cfg_info &info);
    bool guest_normal_cfg_out(base_vcpu *vcpu, cfg_info &info);

    pci_dev(uint32_t addr, struct pci_dev *parent_bridge = nullptr);
    ~pci_dev() = default;
    pci_dev(pci_dev &&) = default;
    pci_dev &operator=(pci_dev &&) = default;
    pci_dev(const pci_dev &) = delete;
    pci_dev &operator=(const pci_dev &) = delete;
};

}
#endif
