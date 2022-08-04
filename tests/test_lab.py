"""Tests for the Lab"""

from pctsp.lab import Lab


def test_lab_init(lab_dir, oplib_root):
    """Test the constructor of the Lab"""
    test_lab = Lab(lab_dir, oplib_root=oplib_root)
    assert test_lab.lab_dir.exists()
