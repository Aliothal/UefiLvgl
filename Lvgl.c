
#include "lvgl.h"
#include "uefiport/lv_port_disp.h"
#include "uefiport/lv_port_indev.h"

static EFI_EVENT ExitEvent;        //you can exit lvgl in anywhere by call gBS->SignalEvent(ExitEvent);

void lv_efi_app_exit(void) {
  gBS->SignalEvent(ExitEvent);
}

static void delayms(UINT32 ms) {
  MicroSecondDelay(ms * 1000);
}

static UINT32 getms(void) {
  return (UINT32)DivU64x32(GetTimeInNanoSecond(GetPerformanceCounter()), 1000000);
}

void EFIAPI LvglTick(EFI_EVENT Event, VOID *Context) {
  lv_tick_inc(1);
  lv_task_handler();
}

void EFIAPI ExitLvgl(EFI_EVENT Event, VOID *Context) {
  
}

int
main(
  IN int Argc,
  IN char *Argv[]
  )
{
  lv_init();
  
  gBS->SetWatchdogTimer (0, 0, 0, NULL);
  EFI_EVENT TickEvent;
  gBS->CreateEvent(EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, (EFI_EVENT_NOTIFY)LvglTick, NULL, &TickEvent);

  gBS->CreateEvent(EVT_NOTIFY_WAIT, TPL_NOTIFY, (EFI_EVENT_NOTIFY)ExitLvgl, NULL, &ExitEvent);

  lv_tick_set_cb(getms);
  lv_delay_set_cb(delayms);

  lv_port_disp_init();
  lv_port_indev_init();
  
  lv_efi_app_main();

  gBS->SetTimer(TickEvent, TimerPeriodic, 10000);           //lvgl periodic task 1ms

  UINTN Index;
  gBS->WaitForEvent(1, &ExitEvent, &Index);                 //block

  gBS->CloseEvent(TickEvent);
  gBS->CloseEvent(ExitEvent);
  lv_port_disp_deinit();
  lv_deinit();
  gST->ConOut->ClearScreen(gST->ConOut);

  return 0;
}