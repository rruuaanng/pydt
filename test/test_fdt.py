import pydt
import unittest


class BaseFDTTests(unittest.TestCase):
    def setUp(self):
        self.fdt = pydt.FDT('test/dt/test.dtb')

    def test_fdt_attr(self):
        self.assertEqual(hex(self.fdt.magic), '0xd00dfeed')
        self.assertIsInstance(self.fdt.version, int)

    def test_fdt_parse_failed(self):
        self.assertRaises(TypeError, pydt.FDT, 1)
        self.assertRaises(TypeError, pydt.FDT, ())
        self.assertRaises(TypeError, pydt.FDT, [])
        self.assertRaises(TypeError, pydt.FDT, {})
        self.assertRaises(RuntimeError, pydt.FDT, '')
        self.assertRaises(RuntimeError, pydt.FDT, 'test/dt/noprefix.dt')
        self.assertRaises(OSError, pydt.FDT, 'test/dt/noexist.dtb')

    def test_fdt_base_methods(self):
        # get_path_offset
        self.assertEqual(self.fdt.get_path_offset('/soc/uart@10000000'), 988)
        self.assertRaises(ValueError, self.fdt.get_path_offset, '/no/uart@000')
        # get_path_props
        props = self.fdt.get_path_props('/soc/uart@10000000')
        self.assertEqual(props['interrupts'][0], '0xa')
        self.assertEqual(props['interrupt-parent'][0], '0x3')
        self.assertEqual(props['clock-frequency'][0], '0x384000')
        self.assertEqual(props['reg'], ['0x0', '0x10000000', '0x0', '0x100'])
        self.assertEqual(props['compatible'], 'ns16550a')
        props = self.fdt.get_path_props('/soc/test@100000')
        self.assertEqual(props['phandle'][0], '0x4')
        self.assertEqual(props['reg'], ['0x0', '0x100000', '0x0', '0x1000'])
        self.assertEqual(props['compatible'], 'sifive,test1\0sifive,test0\0syscon')


if __name__ == "__main__":
    unittest.main()
