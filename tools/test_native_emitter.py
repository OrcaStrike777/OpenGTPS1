#!/usr/bin/env python3
"""Negative tests: generation must fail rather than silently skip instructions."""
import unittest
from generate_mips_tests import backend, BASE, r


class NativeEmitterTests(unittest.TestCase):
    def test_unsupported_instructions(self):
        for word in (0xFFFFFFFF, 0x48000000, 0x40086000, 0x88080000, 0x0000000C):
            with self.subTest(word=hex(word)), self.assertRaises(ValueError):
                backend.emit_function(dict(name="bad", base=BASE, words=[word]))

    def test_invalid_range_and_identifier(self):
        for spec in (dict(name="bad-name", base=BASE, words=[0]),
                     dict(name="bad", base=BASE+1, words=[0]),
                     dict(name="bad", base=BASE, words=[]),
                     dict(name="bad", base=-4, words=[0]),
                     dict(name="bad", base=BASE, words=[0]*4097),
                     dict(name="bad", base=0xFFFFFFFC, words=[0,0])):
            with self.subTest(spec=spec["name"]), self.assertRaises(ValueError):
                backend.emit_function(spec)

    def test_duplicate_symbol(self):
        spec = dict(name="duplicate", base=BASE, words=[0])
        with self.assertRaises(ValueError): backend.emit([spec,spec])

    def test_non_word_input(self):
        for word in (-1, 0x100000000):
            with self.assertRaises(ValueError):
                backend.emit_function(dict(name="bad", base=BASE, words=[word]))

    def test_undefined_jalr_register_pair(self):
        with self.assertRaises(ValueError): backend.operation(r(9,8,8), BASE)

    def test_reserved_bits(self):
        for word in (r(0,8,rs=1,rt=2), r(8,rd=1,rs=31), r(24,rd=1,rs=8,rt=9),
                     r(33,rd=8,rs=9,rt=10,sa=1), 0x3C280001):
            with self.assertRaises(ValueError): backend.operation(word, BASE)


if __name__ == "__main__":
    unittest.main()
