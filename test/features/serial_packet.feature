Feature: Serial Packet Verification
  As a firmware developer
  I want to test serial packet encoding and decoding across multiple languages
  So that I can ensure cross-platform compatibility

  # CMD_PIN_MODE=1, PIN=13, Mode=1(Output)
  Scenario: Encoding set command with pin 13 mode OUTPUT
    Given the buffer is "001 013 001"
    When I encode in "C"
    And I encode in "Python"
    Then the processed buffer must be "006 001 002 013 001 015 000"

  Scenario: Decoding set command with pin 13 mode OUTPUT
    Given the buffer is "006 001 002 013 001 015 00"
    When I decode in "C"
    And I decode in "Python"
    Then the processed buffer must be "001 013 001"
