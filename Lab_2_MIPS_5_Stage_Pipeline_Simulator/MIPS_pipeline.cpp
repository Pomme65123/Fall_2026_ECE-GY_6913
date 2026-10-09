#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab csa23, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct
{
    bitset<32> PC;
    bool nop;
};

struct IDStruct
{
    bitset<32> Instr;
    bool nop;
};

struct EXStruct
{
    bitset<32> Read_data1;
    bitset<32> Read_data2;
    bitset<16> Imm;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool is_I_type;
    bool rd_mem;
    bool wrt_mem;
    bool alu_op; // 1 for addu, lw, sw, 0 for subu
    bool wrt_enable;
    bool nop;
};

struct MEMStruct
{
    bitset<32> ALUresult;
    bitset<32> Store_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;
    bool nop;
};

struct WBStruct
{
    bitset<32> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};

struct stateStruct
{
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
};

class RF
{
public:
    bitset<32> Reg_data;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32>(0);
    }

    bitset<32> readRF(bitset<5> Reg_addr)
    {
        Reg_data = Registers[Reg_addr.to_ulong()];
        return Reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
    {
        Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
    }

    void outputRF(int cycle)
    {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "State of RF (Cycle=" << cycle << ")\t" << endl;

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

    bitset<32> readInstr(bitset<32> ReadAddress)
    {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
        Instruction = bitset<32>(insmem); // read instruction memory
        return Instruction;
    }

private:
    vector<bitset<8>> IMem;
};

class DataMem
{
public:
    bitset<32> ReadData;
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

    bitset<32> readDataMem(bitset<32> Address)
    {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong() + 1].to_string());
        datamem.append(DMem[Address.to_ulong() + 2].to_string());
        datamem.append(DMem[Address.to_ulong() + 3].to_string());
        ReadData = bitset<32>(datamem); // read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData)
    {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
    }

    void outputDataMem()
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

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate << "State after executing cycle:\t" << cycle << endl;

        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        printstate << "ID.Instr:\t" << state.ID.Instr << endl;
        printstate << "ID.nop:\t" << state.ID.nop << endl;

        printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
        printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
        printstate << "EX.Imm:\t" << state.EX.Imm << endl;
        printstate << "EX.Rs:\t" << state.EX.Rs << endl;
        printstate << "EX.Rt:\t" << state.EX.Rt << endl;
        printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
        printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
        printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
        printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
        printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
        printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
        printstate << "EX.nop:\t" << state.EX.nop << endl;

        printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
        printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
        printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
        printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
        printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
        printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
        printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
        printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
        printstate << "MEM.nop:\t" << state.MEM.nop << endl;

        printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
        printstate << "WB.Rs:\t" << state.WB.Rs << endl;
        printstate << "WB.Rt:\t" << state.WB.Rt << endl;
        printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
        printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
        printstate << "WB.nop:\t" << state.WB.nop << endl;
    }
    else
        cout << "Unable to open file";
    printstate.close();
}

int main()
{

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    stateStruct state{}, newState{};

    state.IF.nop = false;
    state.ID.nop = true;
    state.EX.nop = true;
    state.MEM.nop = true;
    state.WB.nop = true;
    state.IF.PC = 0;
    int cycle{};

    while (1)
    {
        newState = state;

        /* --------------------- WB stage --------------------- */

        if (!state.WB.nop)
        {
            if (state.WB.wrt_enable && state.WB.Wrt_reg_addr.to_ulong() != 0)
            {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
        }

        /* --------------------- MEM stage --------------------- */

        newState.WB.nop = state.MEM.nop;
        if (!state.MEM.nop)
        {
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;

            if (state.MEM.rd_mem)
            {
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else if (state.MEM.wrt_mem)
            {
                newState.WB.Wrt_data = state.MEM.Store_data;
            }
            else
            {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }

            if (state.MEM.wrt_mem)
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
        }

        /* --------------------- EX stage --------------------- */

        newState.MEM.nop = state.EX.nop;
        if (!state.EX.nop)
        {
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.rd_mem = state.EX.rd_mem;

            bitset<32> alu_input_1 = state.EX.Read_data1;
            bitset<32> alu_input_2 = state.EX.Read_data2;

            // Rs Forwarding
            if (state.MEM.wrt_enable && state.MEM.Wrt_reg_addr.to_ulong() != 0 && state.MEM.Wrt_reg_addr == state.EX.Rs)
            {
                alu_input_1 = state.MEM.ALUresult;
            }
            else if (state.WB.wrt_enable && state.WB.Wrt_reg_addr.to_ulong() != 0 && state.WB.Wrt_reg_addr == state.EX.Rs)
            {
                alu_input_1 = state.WB.Wrt_data;
            }

            // Rt Forwarding
            if (state.MEM.wrt_enable && state.MEM.Wrt_reg_addr.to_ulong() != 0 && state.MEM.Wrt_reg_addr == state.EX.Rt)
            {
                alu_input_2 = state.MEM.ALUresult;
            }
            else if (state.WB.wrt_enable && state.WB.Wrt_reg_addr.to_ulong() != 0 && state.WB.Wrt_reg_addr == state.EX.Rt)
            {
                alu_input_2 = state.WB.Wrt_data;
            }
            newState.MEM.Store_data = alu_input_2;
            bitset<32> alu_input_2_final = state.EX.is_I_type ? bitset<32>(state.EX.Imm.test(15) ? 0xFFFF0000 | state.EX.Imm.to_ulong() : state.EX.Imm.to_ulong()) : alu_input_2;

            if (state.EX.alu_op)
            {
                newState.MEM.ALUresult = bitset<32>(alu_input_1.to_ulong() + alu_input_2_final.to_ulong());
            }
            else
            {
                newState.MEM.ALUresult = bitset<32>(alu_input_1.to_ulong() - alu_input_2_final.to_ulong());
            }
        }

        /* --------------------- ID stage --------------------- */

        bool isStall{};
        bool isBranchTaken{};
        bitset<32> branchTarget{0};

        newState.EX.nop = state.ID.nop;
        if (!state.ID.nop)
        {
            bitset<32> instr = state.ID.Instr;
            unsigned long inst_val = instr.to_ulong();
            bitset<6> opcode(inst_val >> 26);
            bitset<5> rs((inst_val >> 21) & 0x1F);
            bitset<5> rt((inst_val >> 16) & 0x1F);
            bitset<5> rd((inst_val >> 11) & 0x1F);
            bitset<6> funct(inst_val & 0x3F);
            bitset<16> imm(inst_val & 0xFFFF);

            if (!state.EX.nop && state.EX.rd_mem && (state.EX.Rt == rs || (opcode.to_ulong() == 0x00 && state.EX.Rt == rt)))
            {
                isStall = true;
            }

            if (isStall)
            {
                newState.EX.nop = true;
                newState.ID = state.ID;
                newState.IF = state.IF;
            }
            else
            {
                if (opcode.to_ulong() == 0x00)
                {
                    newState.EX.Rs = rs;
                    newState.EX.Rt = rt;
                    newState.EX.Imm = imm;
                    newState.EX.Read_data1 = myRF.readRF(rs);
                    newState.EX.Read_data2 = myRF.readRF(rt);
                    newState.EX.is_I_type = false;
                    newState.EX.Wrt_reg_addr = rd;
                    newState.EX.wrt_enable = true;
                    newState.EX.rd_mem = false;
                    newState.EX.wrt_mem = false;
                    newState.EX.alu_op = (funct.to_ulong() == 0x21);
                }
                else if (opcode.to_ulong() == 0x23)
                {
                    newState.EX.Rs = rs;
                    newState.EX.Rt = rt;
                    newState.EX.Imm = imm;
                    newState.EX.Read_data1 = myRF.readRF(rs);
                    newState.EX.Read_data2 = myRF.readRF(rt);
                    newState.EX.is_I_type = true;
                    newState.EX.Wrt_reg_addr = rt;
                    newState.EX.wrt_enable = true;
                    newState.EX.rd_mem = true;
                    newState.EX.wrt_mem = false;
                    newState.EX.alu_op = true;
                }
                else if (opcode.to_ulong() == 0x2B)
                {
                    newState.EX.Rs = rs;
                    newState.EX.Rt = rt;
                    newState.EX.Imm = imm;
                    newState.EX.Read_data1 = myRF.readRF(rs);
                    newState.EX.Read_data2 = myRF.readRF(rt);
                    newState.EX.is_I_type = true;
                    newState.EX.Wrt_reg_addr = rt;
                    newState.EX.wrt_enable = false;
                    newState.EX.rd_mem = false;
                    newState.EX.wrt_mem = true;
                    newState.EX.alu_op = true;
                }
                else if (opcode.to_ulong() == 0x05)
                {
                    newState.EX.Rs = rs;
                    newState.EX.Rt = rt;
                    newState.EX.Imm = imm;
                    newState.EX.Read_data1 = myRF.readRF(rs);
                    newState.EX.Read_data2 = myRF.readRF(rt);
                    newState.EX.nop = true;
                    newState.EX.wrt_enable = false;
                    newState.EX.rd_mem = false;
                    newState.EX.wrt_mem = false;

                    if (newState.EX.Read_data1 != newState.EX.Read_data2)
                    {
                        isBranchTaken = true;
                        long imm_val = imm.test(15) ? (0xFFFF0000 | imm.to_ulong()) : imm.to_ulong();
                        branchTarget = bitset<32>(state.IF.PC.to_ulong() + (imm_val << 2));
                    }
                }
                else if (opcode.to_ulong() == 0x3F)
                {
                    newState.EX.nop = true;
                }
            }
        }

        /* --------------------- IF stage --------------------- */

        if (!isStall)
        {
            if (isBranchTaken)
            {
                newState.IF.PC = branchTarget;
                newState.ID.nop = true;
            }
            else if (!state.IF.nop)
            {
                bitset<32> inst = myInsMem.readInstr(state.IF.PC);
                newState.ID.Instr = inst;
                newState.ID.nop = false;

                if (inst.to_ulong() >> 26 == 0x3F)
                {
                    newState.IF.nop = true;
                    newState.ID.nop = true;
                }
                else
                {
                    newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
                }
            }
            else
            {
                newState.ID.nop = true;
            }
        }

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;

        printState(newState, cycle); // print states after executing cycle 0, cycle 1, cycle 2 ...

        state = newState; /*** The end of the cycle and updates the current state with the values calculated in this cycle. csa23 ***/

        myRF.outputRF(cycle); // dump RF;

        cycle++;
    }

    myDataMem.outputDataMem(); // dump data mem

    return 0;
}

// a06ab8e681fd21c64d696c93bec4b1ef