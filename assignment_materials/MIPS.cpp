#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>

using namespace std;

// Internal ALU control codes.
#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR (5)
#define NOR (7)

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize (65536)

class RF
{
public:
  bitset<32> ReadData1, ReadData2;
  RF()
  {
    Registers.resize(32);
    Registers[0] = bitset<32>(0);
  }

  void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
  {
    /**
     * @brief Reads or writes data from/to the Register.
     *
     * This function is used to read or write data from/to the register, depending on the value of WrtEnable.
     * Put the read results to the ReadData1 and ReadData2.
     */
    // TODO: implement!
    if (WrtEnable == true)
    {
      if (WrtReg.to_ulong() == 0)
      {
      } // Register 0x0 is hardcoded to 0
      else
      {
        Registers[WrtReg.to_ulong()] = WrtData;
      }
    }

    ReadData1 = Registers[RdReg1.to_ulong()];
    ReadData2 = Registers[RdReg2.to_ulong()];
  }

  void OutputRF()
  {
    ofstream rfout;
    rfout.open("RFresult.txt", std::ios_base::app);
    if (rfout.is_open())
    {
      rfout << "A state of RF:" << endl;
      for (int j = 0; j < 32; j++)
      {
        rfout << Registers[j] << endl;
      }
    }
    else
      cout << "Unable to open file";
    rfout.close();
  }

private:
  vector<bitset<32>> Registers;
};

class ALU
{
public:
  bitset<32> ALUresult;
  bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
  {
    /**
     * @brief Implement the ALU operation here.
     *
     * ALU operation depends on the ALUOP, which are definded as ADDU, SUBU, etc.
     */
    // TODO: implement!

    // #define ADDU (1)
    // #define SUBU (3)
    // #define AND (4)
    // #define OR  (5)
    // #define NOR (7)

    switch (ALUOP.to_ulong())
    {
    case (ADDU):
      ALUresult = bitset<32>(oprand1.to_ulong() + oprand2.to_ulong());
      break;
    case (SUBU):
      ALUresult = bitset<32>(oprand1.to_ulong() - oprand2.to_ulong());
      break;
    case (AND):
      ALUresult = bitset<32>(oprand1.to_ulong() & oprand2.to_ulong());
      break;
    case (OR):
      ALUresult = bitset<32>(oprand1.to_ulong() | oprand2.to_ulong());
      break;
    case (NOR):
      ALUresult = bitset<32>(~(oprand1.to_ulong() | oprand2.to_ulong()));
      break;
    default:
      break;
    }

    return ALUresult;
  }
};

class INSMem
{
public:
  bitset<32> Instruction;
  INSMem()
  {
    IMem.resize(MemSize);
    ifstream imem;
    string line;
    int i = 0;
    imem.open("imem.txt");
    if (imem.is_open())
    {
      while (getline(imem, line))
      {
        IMem[i] = bitset<8>(line);
        i++;
      }
    }
    else
      cout << "Unable to open file";
    imem.close();
  }

  bitset<32> ReadMemory(bitset<32> ReadAddress)
  {
    // TODO: implement!
    /**
     * @brief Read Instruction Memory (IMem).
     *
     * Read the byte at the ReadAddress and the following three byte,
     * and return the read result.
     */

    Instruction.reset();
    for (int i{}; i < 4; i++)
    {
      Instruction <<= 8;
      Instruction |= bitset<32>(IMem[ReadAddress.to_ulong() + i].to_ulong());
    }

    return Instruction;
  }

private:
  vector<bitset<8>> IMem;
};

class DataMem
{
public:
  bitset<32> readdata;
  DataMem()
  {
    DMem.resize(MemSize);
    ifstream dmem;
    string line;
    int i = 0;
    dmem.open("dmem.txt");
    if (dmem.is_open())
    {
      while (getline(dmem, line))
      {
        DMem[i] = bitset<8>(line);
        i++;
      }
    }
    else
      cout << "Unable to open file";
    dmem.close();
  }
  bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
  {
    /**
     * @brief Reads/writes data from/to the Data Memory.
     *
     * This function is used to read/write data from/to the DataMem, depending on the readmem and writemem.
     * First, if writemem enabled, WriteData should be written to DMem, clear or ignore the return value readdata,
     * and note that 32-bit WriteData will occupy 4 continious Bytes in DMem.
     * If readmem enabled, return the DMem read result as readdata.
     */
    if (writemem == true)
    {
      for (int i{}; i < 4; i++)
      {
        bitset<8> byte_value((WriteData.to_ulong() >> (8 * (3 - i))) & 0xFF);
        DMem[Address.to_ulong() + i] = byte_value;
      }
      readdata.reset();
    }

    if (readmem == true)
    {
      readdata.reset();
      for (int i{}; i < 4; i++)
      {
        readdata <<= 8;
        readdata |= bitset<32>(DMem[Address.to_ulong() + i].to_ulong());
      }
    }

    return readdata;
  }

  void OutputDataMem()
  {
    ofstream dmemout;
    dmemout.open("dmemresult.txt");
    if (dmemout.is_open())
    {
      for (int j = 0; j < 1000; j++)
      {
        dmemout << DMem[j] << endl;
      }
    }
    else
      cout << "Unable to open file";
    dmemout.close();
  }

private:
  vector<bitset<8>> DMem;
};

struct Instruction_Fields
{
  // R-types
  bitset<6> opcode{};
  bitset<5> rs{};
  bitset<5> rt{};
  bitset<5> rd{};
  bitset<5> shamt{};
  bitset<6> funct{};

  // I-types
  bitset<16> imm{};

  // J-types
  bitset<26> addr{};

  Instruction_Fields() = default;

  void get_fields(bitset<32> instr)
  {
    constexpr unsigned long MASK5 = 0x1F;
    constexpr unsigned long MASK6 = 0x3F;
    constexpr unsigned long MASK16 = 0xFFFF;
    constexpr unsigned long MASK26 = 0x3FFFFFF;

    unsigned long converted_instr = instr.to_ulong();

    opcode = bitset<6>(converted_instr >> 26);
    rs = bitset<5>((converted_instr >> 21) & MASK5);
    rt = bitset<5>((converted_instr >> 16) & MASK5);
    rd = bitset<5>((converted_instr >> 11) & MASK5);
    shamt = bitset<5>((converted_instr >> 6) & MASK5);
    funct = bitset<6>(converted_instr & MASK6);
    imm = bitset<16>(converted_instr & MASK16);
    addr = bitset<26>(converted_instr & MASK26);
  }
};

struct Decoder
{
  bool RegDst{};
  bool ALUSrc{};
  bool ExtSel{};
  bool MemRead{};
  bool MemWrite{};
  bool MemtoReg{};
  bool RegWrite{};
  bitset<3> ALUCntrl{};
  bool isJ{};
  bool isBeq{};

  Decoder() = default;

  void set_Decoder(bitset<6> const opcode, bitset<6> const funct)
  {

    unsigned long converted_opcode = opcode.to_ulong();

    isR_ALU = (converted_opcode == 0x00) ? true : false;
    isI_ALU = (converted_opcode == 0x09);

    isLW = (converted_opcode == 0x23) ? true : false;
    isSW = (converted_opcode == 0x2B) ? true : false;

    isBeq = (converted_opcode == 0x04) ? true : false;
    isJ = (converted_opcode == 0x02) ? true : false;

    RegDst = (isR_ALU) ? true : false; // (rd/rt)

    ALUSrc = (isI_ALU || isLW || isSW);

    /*
        https://checkoway.net/teaching/cs210/2021-fall/MIPS_Green_Sheet.pdf
        0xc : ANDI
        0xd : ORI
        0x8 : ADDI
        0x9 : ADDIU

    */

    if (converted_opcode == 0xc || converted_opcode == 0xd)
    { // ANDI and ORI opcodes
      ExtSel = true;
    }
    else if (converted_opcode == 0x8 || converted_opcode == 0x9 || isLW || isSW || isBeq)
    {
      ExtSel = false;
    }

    MemRead = isLW;
    MemWrite = isSW;
    MemtoReg = isLW;
    RegWrite = isR_ALU || isI_ALU || isLW;

    if (isLW || isSW)
    {
      ALUCntrl = ADDU;
    }
    else if (isR_ALU)
    {
      ALUCntrl = bitset<3>(funct.to_ulong() & 0x7);
    }
    else if (isI_ALU)
    {
      ALUCntrl = bitset<3>(converted_opcode & 0x7);
    }
    else if (isBeq)
    {
      ALUCntrl = SUBU;
    }
  }

private:
  bool isR_ALU{};
  bool isI_ALU{};

  bool isLW{};
  bool isSW{};

  // bool isBeq{};
  // bool isJ{};
};

bitset<32> Imm_Extend(bitset<16> value_imm)
{ // https://en.wikipedia.org/wiki/Sign_extension
  if (value_imm.test(15))
  {
    return bitset<32>(value_imm.to_ulong() | 0xFFFF0000);
  }
  else
  {
    return bitset<32>(value_imm.to_ulong());
  }
}

int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;
  bitset<32> PC(0); // 32-bit program counter, initialized to 0

  Decoder myDecoder;
  Instruction_Fields myInstruction_Fields;

  unsigned long instr{};

  // MUXES
  bitset<32> MUX_NextPCSel{};
  bitset<5> MUX_RegDst{};
  bitset<32> MUX_ALUSrc{};
  bitset<32> MUX_MemtoReg{};

  while (1) // TODO: implement!
  {
    // Fetch: fetch an instruction from myInsMem.
    instr = myInsMem.ReadMemory(PC).to_ulong();

    // If current instruction is "11111111111111111111111111111111", then break; (exit the while loop)
    if (instr == 0xFFFFFFFF)
    {
      break;
    }

    // decode(Read RF): get opcode and other signals from instruction, decode instruction
    bitset<32> bitset_instr(instr);
    myInstruction_Fields.get_fields(bitset_instr);
    myDecoder.set_Decoder(myInstruction_Fields.opcode, myInstruction_Fields.funct);

    MUX_RegDst = (myDecoder.RegDst) ? myInstruction_Fields.rd : myInstruction_Fields.rt;

    myRF.ReadWrite(myInstruction_Fields.rs, myInstruction_Fields.rt,
                   bitset<5>(0), bitset<32>(0), bitset<1>(0)); // Forcing initial read
    bitset<32> branch_data1 = myRF.ReadData1;
    bitset<32> branch_data2 = myRF.ReadData2;

    // Execute: after decoding, ALU may run and return result
    bitset<32> extended_imm = Imm_Extend(myInstruction_Fields.imm);
    MUX_ALUSrc = (myDecoder.ALUSrc) ? extended_imm : myRF.ReadData2;
    myALU.ALUOperation(myDecoder.ALUCntrl, myRF.ReadData1, MUX_ALUSrc);

    // Read/Write Mem: access data memory (myDataMem)
    myDataMem.MemoryAccess(myALU.ALUresult, myRF.ReadData2, bitset<1>(myDecoder.MemRead), bitset<1>(myDecoder.MemWrite));

    // Write back to RF: some operations may write things to RF
    MUX_MemtoReg = (myDecoder.MemtoReg) ? myDataMem.readdata : myALU.ALUresult;
    myRF.ReadWrite(bitset<5>(0), bitset<5>(0), MUX_RegDst, MUX_MemtoReg, bitset<1>(myDecoder.RegWrite));

    // Update PC

    if (myDecoder.isBeq)
    {
      if (branch_data1 == branch_data2)
      {
        bitset<32> branch_offset = Imm_Extend(myInstruction_Fields.imm);
        branch_offset = bitset<32>(branch_offset.to_ulong() << 2);
        PC = bitset<32>(PC.to_ulong() + 4 + branch_offset.to_ulong());
      }
      else
      {
        PC = bitset<32>(PC.to_ulong() + 4);
      }
    }
    else if (myDecoder.isJ)
    {
      bitset<32> PC_plus_4(PC.to_ulong() + 4);
      bitset<32> new_PC(0);

      unsigned long upper_bits = (PC_plus_4.to_ulong() >> 28) & 0xF;
      unsigned long addr_shifted = (myInstruction_Fields.addr.to_ulong() << 2) & 0x0FFFFFFF;

      new_PC = bitset<32>((upper_bits << 28) | addr_shifted);
      PC = new_PC;
    }
    else
    {
      PC = bitset<32>(PC.to_ulong() + 4);
    }

    /**** You don't need to modify the following lines. ****/
    myRF.OutputRF(); // dump RF;
  }
  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
