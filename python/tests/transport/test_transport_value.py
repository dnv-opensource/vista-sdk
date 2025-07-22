"""Tests for the transport value module."""

from datetime import datetime, timezone

import pytest

from vista_sdk.transport.value import (  # type: ignore[import-untyped,import-not-found]
    BooleanValue,
    CharValue,
    DateTimeValue,
    DecimalValue,
    DoubleValue,
    IntegerValue,
    LongValue,
    StringValue,
    UnsignedIntegerValue,
    Value,
)


def test_string_value() -> None:
    """Test StringValue creation and operations."""
    value = StringValue("test string")
    assert value.value == "test string"
    assert str(value) == "StringValue('test string')"

    # Test equality
    value2 = StringValue("test string")
    value3 = StringValue("different")
    assert value == value2
    assert value != value3

    # Test hash
    assert hash(value) == hash(value2)
    assert hash(value) != hash(value3)


def test_char_value() -> None:
    """Test CharValue creation and operations."""
    value = CharValue("x")
    assert value.value == "x"
    # Test invalid character
    with pytest.raises(ValueError, match="exactly one character"):
        CharValue("abc")


def test_boolean_value() -> None:
    """Test BooleanValue creation and operations."""
    value_true = BooleanValue(True)
    value_false = BooleanValue(False)

    assert value_true.value is True
    assert value_false.value is False

    assert value_true != value_false
    assert hash(value_true) != hash(value_false)


def test_integer_value() -> None:
    """Test IntegerValue creation and operations."""
    value = IntegerValue(42)
    assert value.value == 42
    assert str(value) == "IntegerValue(42)"


def test_unsigned_integer_value() -> None:
    """Test UnsignedIntegerValue creation and operations."""
    value = UnsignedIntegerValue(42)
    # Test negative value
    with pytest.raises(ValueError, match="non-negative"):
        UnsignedIntegerValue(-1)
    assert value.value == 9223372036854775807


def test_double_value() -> None:
    """Test DoubleValue creation and operations."""
    value = DoubleValue(3.14159)
    assert value.value == 3.14159


def test_decimal_value() -> None:
    """Test DecimalValue creation and operations."""
    value = DecimalValue(123.456)
    assert value.value == 123.456


def test_datetime_value() -> None:
    """Test DateTimeValue creation and operations."""
    dt = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    value = DateTimeValue(dt)
    assert value.value == dt


def test_value_inheritance() -> None:
    """Test that all value types inherit from Value."""
    values = [
        StringValue("test"),
        CharValue("x"),
        BooleanValue(True),
        IntegerValue(42),
        UnsignedIntegerValue(42),
        LongValue(42),
        DoubleValue(3.14),
        DecimalValue(123.45),
        DateTimeValue(datetime.now(timezone.utc)),
    ]

    for value in values:
        assert isinstance(value, Value)


def test_value_equality_different_types() -> None:
    """Test that different value types are not equal."""
    string_val = StringValue("42")
    int_val = IntegerValue(42)

    assert string_val != int_val
    assert hash(string_val) != hash(int_val)


def test_value_representations() -> None:
    """Test string representations of all value types."""
    dt = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)

    values_and_reprs = [
        (StringValue("hello"), "StringValue('hello')"),
        (CharValue("a"), "CharValue('a')"),
        (BooleanValue(True), "BooleanValue(True)"),
        (IntegerValue(123), "IntegerValue(123)"),
        (UnsignedIntegerValue(456), "UnsignedIntegerValue(456)"),
        (LongValue(789), "LongValue(789)"),
        (DoubleValue(1.23), "DoubleValue(1.23)"),
        (DecimalValue(4.56), "DecimalValue(4.56)"),
        (DateTimeValue(dt), f"DateTimeValue({dt!r})"),
    ]

    for value, expected_repr in values_and_reprs:
        assert repr(value) == expected_repr
