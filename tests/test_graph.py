"""Test graph fixtures"""


def test_grid8(grid8):
    """Read grid8 graph from dot file"""
    assert grid8.number_of_nodes() == 8
    assert grid8.number_of_edges() == 10
    assert grid8[1][4]["cost"] == 5
