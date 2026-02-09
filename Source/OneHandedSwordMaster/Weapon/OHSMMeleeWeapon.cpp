// Fill out your copyright notice in the Description page of Project Settings.


#include "OHSMMeleeWeapon.h"

AOHSMMeleeWeapon::AOHSMMeleeWeapon()
{
	WeaponType = EOHSMWeaponType::OneHandSword;
	WeaponName = FText::FromString(TEXT("Iron Sword"));
	BaseDamage = 30.0f;
}
