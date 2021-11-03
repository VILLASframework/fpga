
// instantiate factory to register
static PCIeCardFactory villas::fpga::PCIeCardFactory;

Card::Ptr
PCIeCardFactory::make(json_t *json, std::shared_ptr<kernel::pci::DeviceList> pci, std::shared_ptr<kernel::vfio::Container> vc)
{
	const char *pci_slot = nullptr;
	const char *pci_id = nullptr;

	int ret = json_unpack_ex(json, &err, "{ s?: s, s?: s }",
		"slot", &pci_slot,
		"id", &pci_id
	);
	if (ret != 0)
		throw ConfigError(json_card, err, "node-config-fpgas", "Failed to parse FPGA configuration");

	kernel::pci::Device filter = defaultFilter;
		
	if (pci_id)
		filter.id = kernel::pci::Id(pci_id);
	if (pci_slot)
		filter.slot = kernel::pci::Slot(pci_slot);

	/* Search for FPGA card */
	card->pdev = pci->lookupDevice(filter);
	if (!card->pdev)
		throw RuntimeError("Failed to find PCI device");
}


bool
PCIeCard::init()
{
	logger = getLogger();

	logger->info("Initializing FPGA card {}", name);

	/* Attach PCIe card to VFIO container */
	kernel::vfio::Device &device = vfioContainer->attachDevice(*pdev);
	this->vfioDevice = &device;

	/* Enable memory access and PCI bus mastering for DMA */
	if (not device.pciEnable()) {
		logger->error("Failed to enable PCI device");
		return false;
	}

	/* Reset system? */
	if (doReset) {
		/* Reset / detect PCI device */
		if (not vfioDevice->pciHotReset()) {
			logger->error("Failed to reset PCI device");
			return false;
		}

		if (not reset()) {
			logger->error("Failed to reset FGPA card");
			return false;
		}
	}

	return true;
}
