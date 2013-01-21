#ifndef PCI_FUNCTION_H
#define PCI_FUNCTION_H

#include <map>
#include <ostream>

#include "types.hh"
#include "processor.hh"
#include "debug.hh"

using namespace processor;
using namespace std;

namespace pci {

	class pci_function;

	class pci_bar {
	public:

		enum pci_bar_encoding_masks {
			// mmio, pio
			PCI_BAR_MEMORY_INDICATOR_MASK 	= (1 << 0),
			// 32bit, 64bit
			PCI_BAR_MEM_ADDR_SPACE_MASK		= (1 << 1) | (1 << 2),
			PCI_BAR_PREFETCHABLE_MASK		= (1 << 3),
			PCI_BAR_MEM_ADDR_LO_MASK		= 0xFFFFFFF0,
			PCI_BAR_PIO_ADDR_MASK			= 0xFFFFFFFC,
		};

		enum pci_bar_type_indicator {
			PCI_BAR_MMIO 				= 0x00,
			PCI_BAR_PIO 				= 0x01
		};

		enum pci_bar_prefetchable {
			PCI_BAR_NON_PREFETCHABLE	= 0x00,
			PCI_BAR_PREFETCHABLE		= 0x01
		};

		enum pci_bar_address_space {
			PCI_BAR_32BIT_ADDRESS		= 0x00,
			PCI_BAR_64BIT_ADDRESS		= 0x01
		};

		pci_bar(pci_function* dev, u8 pos);
		virtual ~pci_bar();

		// pos is the offset within the configuration space
		void test_bar_size(void);

		bool is_pio(void) { return (!_is_mmio); }
		bool is_mmio(void) { return (_is_mmio); }
		bool is_32(void) { return (!_is_64); }
		bool is_64(void) { return (_is_64); }
		bool is_prefetchable(void) { return (_is_prefetchable); }

		u32 get_addr_lo(void) { return (_addr_lo); }
		u32 get_addr_hi(void) { return (_addr_hi); }
		u64 get_addr64(void) { return (_addr_64); }
		u64 get_size(void) { return (_addr_size); }

		// Access the pio bar
	    u32 read(u32 offset)  { return (inl(_addr_lo + offset)); }
	    u16 readw(u32 offset) { return (inw(_addr_lo + offset)); }
	    u8  readb(u32 offset) { return (inb(_addr_lo + offset)); }
	    void write(u32 offset, u32 val) { outl(val, _addr_lo+offset); }
	    void write(u32 offset, u16 val) { outw(val, _addr_lo+offset); }
	    void write(u32 offset, u8 val)  { outb(val, _addr_lo+offset); }

	private:

	    void init(void);

	    // To which pci_function it relates
	    pci_function* _dev;
		// Offset to configuration space
		u8 _pos;
		// Base address
		u32 _addr_lo, _addr_hi;
		u64 _addr_64;
		u64 _addr_size;

		bool _is_mmio;
		bool _is_64;
		bool _is_prefetchable;
	};

	//  MSI-X definitions
	enum msix_pci_conf {
		PCIR_MSIX_CTRL = 0x2,
		PCIM_MSIXCTRL_MSIX_ENABLE = 0x8000,
		PCIM_MSIXCTRL_FUNCTION_MASK = 0x4000,
		PCIM_MSIXCTRL_TABLE_SIZE = 0x07FF,
		PCIR_MSIX_TABLE = 0x4,
		PCIR_MSIX_PBA = 0x8,
		PCIM_MSIX_BIR_MASK = 0x7,
		PCIM_MSIX_BIR_BAR_10 = 0,
		PCIM_MSIX_BIR_BAR_14 = 1,
		PCIM_MSIX_BIR_BAR_18 = 2,
		PCIM_MSIX_BIR_BAR_1C = 3,
		PCIM_MSIX_BIR_BAR_20 = 4,
		PCIM_MSIX_BIR_BAR_24 = 5,
		PCIM_MSIX_VCTRL_MASK = 0x1
	};

	//  Interesting values for PCI MSI-X
	struct msix_vector {
		u64 mv_address;         // Contents of address register.
		u32 mv_data;            // Contents of data register.
		int     mv_irq;
	};

	struct msix_table_entry {
		u32   mte_vector; //  1-based index into msix_vectors array.
		u32   mte_handlers;
	};

	struct pcicfg_msix {
		u16 msix_ctrl;                          //  Message Control
		u16 msix_msgnum;                        //  Number of messages
		u8 msix_location;                       //  Offset of MSI-X capability registers.
		u8 msix_table_bar;                      //  BAR containing vector table.
		u8 msix_pba_bar;                        //  BAR containing PBA.
		u32 msix_table_offset;
		u32 msix_pba_offset;
		int msix_alloc;                         //  Number of allocated vectors.
		int msix_table_len;                     //  Length of virtual table.
		struct msix_table_entry* msix_table;    //  Virtual table.
		struct msix_vector* msix_vectors;       //  Array of allocated vectors.
		pci_bar* msix_table_res;                //  Resource containing vector table.
		pci_bar* msix_pba_res;                  //  Resource containing PBA.
	};

	// Represents a PCI function (pci device, bridge or virtio device)
	class pci_function {
	public:

		enum pci_function_cfg_offsets {
			PCI_CFG_VENDOR_ID 			= 0x00,
			PCI_CFG_DEVICE_ID 			= 0x02,
			PCI_CFG_COMMAND 			= 0x04,
			PCI_CFG_STATUS 				= 0x06,
			PCI_CFG_REVISION_ID 		= 0x08,
			PCI_CFG_CLASS_CODE0			= 0x09,
			PCI_CFG_CLASS_CODE1			= 0x0A,
			PCI_CFG_CLASS_CODE2			= 0x0B,
			PCI_CFG_CACHELINE_SIZE		= 0x0C,
			PCI_CFG_LATENCY_TIMER 		= 0x0D,
			PCI_CFG_HEADER_TYPE 		= 0x0E,
			PCI_CFG_BIST 				= 0x0F,
			PCI_CFG_BAR_1		 		= 0x10,
			PCI_CFG_BAR_2		 		= 0x14,
			PCI_CFG_BAR_3		 		= 0x18,
			PCI_CFG_BAR_4		 		= 0x1C,
			PCI_CFG_BAR_5		 		= 0x20,
			PCI_CFG_BAR_6		 		= 0x24,
			PCI_CFG_CARDBUS_CIS_PTR 	= 0x28,
			PCI_CFG_SUBSYSTEM_VENDOR_ID = 0x2C,
			PCI_CFG_SUBSYSTEM_ID 		= 0x2E,
			PCI_CFG_CAPABILITIES_PTR 	= 0x34,
			PCI_CFG_INTERRUPT_LINE 		= 0x3C,
			PCI_CFG_INTERRUPT_PIN		= 0x3D
		};

		enum pci_command_bits {
		    PCI_COMMAND_INTX_DISABLE = (1 << 10),
		    PCI_COMMAND_BUS_MASTER = (1 << 2)
		};

		enum pci_header_type {
			PCI_HDR_TYPE_DEVICE = 0x00,
			PCI_HDR_TYPE_BRIDGE = 0x01,
			PCI_HDR_TYPE_PCCARD = 0x02
		};

		//  Capability Register Offsets
		enum pci_capabilities_offsets {
		    PCI_CAP_OFF_ID      = 0x0,
		    PCI_CAP_OFF_NEXT    = 0x1
		};

		enum pci_capabilities {
		    PCI_CAP_PM          = 0x01,    // PCI Power Management
		    PCI_CAP_AGP         = 0x02,    // AGP
		    PCI_CAP_VPD         = 0x03,    // Vital Product Data
		    PCI_CAP_SLOTID      = 0x04,    // Slot Identification
		    PCI_CAP_MSI         = 0x05,    // Message Signaled Interrupts
		    PCI_CAP_CHSWP       = 0x06,    // CompactPCI Hot Swap
		    PCI_CAP_PCIX        = 0x07,    // PCI-X
		    PCI_CAP_HT          = 0x08,    // HyperTransport
		    PCI_CAP_VENDOR      = 0x09,    // Vendor Unique
		    PCI_CAP_DEBUG       = 0x0a,    // Debug port
		    PCI_CAP_CRES        = 0x0b,    // CompactPCI central resource control
		    PCI_CAP_HOTPLUG     = 0x0c,    // PCI Hot-Plug
		    PCI_CAP_SUBVENDOR   = 0x0d,    // PCI-PCI bridge subvendor ID
		    PCI_CAP_AGP8X       = 0x0e,    // AGP 8x
		    PCI_CAP_SECDEV      = 0x0f,    // Secure Device
		    PCI_CAP_EXPRESS     = 0x10,    // PCI Express
		    PCI_CAP_MSIX        = 0x11,    // MSI-X
		    PCI_CAP_SATA        = 0x12,    // SATA
		    PCI_CAP_PCIAF       = 0x13     // PCI Advanced Features
		};

		pci_function(u8 bus, u8 device, u8 func);
		virtual ~pci_function();

		// Parses the PCI configuration space
		virtual bool parse_pci_config(void);

		// Position
	    void get_bdf(u8& bus, u8 &device, u8& func);
	    void set_bdf(u8 bus, u8 device, u8 func);

	    // Identification
	    u16 get_vendor_id(void);
	    u16 get_device_id(void);
	    u8 get_revision_id(void);

	    // Type
	    bool is_device(void);
	    bool is_bridge(void);
	    bool is_pccard(void);

	    static bool is_device(u8 bus, u8 device, u8 function);
	    static bool is_bridge(u8 bus, u8 device, u8 function);
	    static bool is_pccard(u8 bus, u8 device, u8 function);

	    // Command & Status
	    u16 get_command(void);
	    u16 get_status(void);
	    void set_command(u16 command);
	    void set_status(u16 status);

	    bool get_bus_master();
	    void set_bus_master(bool m);

	    // Enable/Disable intx assertions
	    bool is_intx_enabled(void);
	    // Enable intx assertion
	    // intx assertions should be disabled in order to use MSI-x
	    void enable_intx(void);
	    void disable_intx(void);

	    // Interrupts (PCI configuration)
	    u8 get_interrupt_line(void);
	    void set_interrupt_line(u8 irq);
	    u8 get_interrupt_pin(void);

	    bool is_msix(void);

	    // Access to PCI address space
	    virtual u8 pci_readb(u8 offset);
	    virtual u16 pci_readw(u8 offset);
	    virtual u32 pci_readl(u8 offset);
	    virtual void pci_writeb(u8 offset, u8 val);
	    virtual void pci_writew(u8 offset, u16 val);
	    virtual void pci_writel(u8 offset, u32 val);

	    // Capability parsing
	    u8 find_capability(u8 cap_id);

	    pci_bar * get_bar(int idx);
	    void add_bar(int idx, pci_bar * bar);

	    // Useful function to print device
	    virtual void dump_config(void);

	    friend std::ostream& operator << (std::ostream& out, const pci_function &d);
	    struct equal {
			bool operator()(const pci_function* d1, const pci_function* d2) const
			{
				return (d1->_device_id == d2->_device_id && d1->_vendor_id == d2->_vendor_id);
			}
	    };

		struct hash : std::unary_function< const pci_function*, std::size_t> {
			std::size_t operator() ( const pci_function* const key ) const {
				return (size_t)((key->_device_id<<16)+ key->_vendor_id);
			}
		};

	protected:
    	// Parsing of extra capabilities
    	virtual bool parse_pci_capabilities(void);
		virtual bool parse_pci_msix(u8 off);

		// Position
		u8  _bus, _device, _func;

		// Some fields that are parsed from the configuration space
		u16 _device_id;
	    u16 _vendor_id;
	    u8 _revision_id;
	    u8 _header_type;
	    u8 _base_class_code;
	    u8 _sub_class_code;
	    u8 _lower_class_code;

	    // Index -> PCI Bar
	    std::map<int, pci_bar *> _bars;

	    // MSI-x
	    bool _have_msix;
	    pcicfg_msix _msix;
	};
}

#endif // PCI_DEVICE_H
