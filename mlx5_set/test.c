#include <stdio.h>

struct mlx5_ifc_create_flow_group_in_bits {
        char         opcode[0x10];
        char         reserved_at_10[0x10];

        char         reserved_at_20[0x10];
        char         op_mod[0x10];

        char         other_vport[0x1];
        char         reserved_at_41[0xf];
        char         vport_number[0x10];

        char         reserved_at_60[0x20];

        char         table_type[0x8];
        char         reserved_at_88[0x18];

        char         reserved_at_a0[0x8];
        char         table_id[0x18];
};

#define __mlx5_nullp(typ) ((struct mlx5_ifc_##typ##_bits *)0)
#define __mlx5_bit_sz(typ, fld) sizeof(__mlx5_nullp(typ)->fld)
#define __mlx5_bit_off(typ, fld) ((unsigned)(unsigned long)(&(__mlx5_nullp(typ)->fld)))
#define __mlx5_dw_off(typ, fld) (__mlx5_bit_off(typ, fld) / 32)
#define __mlx5_64_off(typ, fld) (__mlx5_bit_off(typ, fld) / 64)
#define __mlx5_dw_bit_off(typ, fld) (32 - __mlx5_bit_sz(typ, fld) - (__mlx5_bit_off(typ, fld) & 0x1f))
#define __mlx5_mask(typ, fld) ((uint32_t)((1ull << __mlx5_bit_sz(typ, fld)) - 1))
#define __mlx5_dw_mask(typ, fld) (__mlx5_mask(typ, fld) << __mlx5_dw_bit_off(typ, fld))
#define __mlx5_st_sz_bits(typ) sizeof(struct mlx5_ifc_##typ##_bits)

#define TEST_FIELD	vport_number
#define TEST_FIELD	other_vport

int main(int argc, char *argv[])
{
	struct mlx5_ifc_create_flow_group_in_bits in;
	uint32_t *flow_group_in = (uint32_t *)&in;
	printf("hello, test, %x\n", __mlx5_bit_sz(create_flow_group_in, TEST_FIELD));
	printf("hello, test, %x\n", __mlx5_bit_off(create_flow_group_in, TEST_FIELD));
	printf("hello, test, %x\n", __mlx5_dw_mask(create_flow_group_in, TEST_FIELD));
	printf("hello, test, %x\n", ~__mlx5_dw_mask(create_flow_group_in, TEST_FIELD));
	printf("hello, test, %d\n", __mlx5_dw_bit_off(create_flow_group_in, TEST_FIELD));

	return 0;
}
