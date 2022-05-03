Instruction of migration from NATS to GNATS

Users who work on old versions of NATS have to make changes in their programs in order to use GNATS library.

The new GNATS Standalone initialization statement is(example code in Python):
	clsGNATSStandalone = JClass('GNATSStandalone')
	gnatsStandalone = clsGNATSStandalone.start()

GNATS entry point is provided in the class GNATSStandalone, not NATSStandalone.
Please change the class name accordingly.
