Instruction of migration from NATS to GNATS

Users who work on old versions of NATS have to make changes in their programs in order to use GNATS library.

The new GNATS Client initialization statement is(example code in Python):
	GNATSClientFactory = JClass('GNATSClientFactory')
	gnatsClient = GNATSClientFactory.getGNATSClient()

Please change the class name accordingly.
