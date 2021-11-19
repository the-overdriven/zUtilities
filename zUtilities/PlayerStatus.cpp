// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  void PlayerStatus::ResetTimeMultiplier() {
    if ( !ztimer )
      return;

    ztimer->factorMotion = 1.0f;
  }

  void PlayerStatus::FactorMotion() {
    if ( !Options::UseTimeMultiplier )
      return;

    if ( !Options::TimeMultipliers.GetNum() || zcon->IsVisible() || (edit_con && edit_con->IsVisible()) )
      return;

    if ( playerHelper.IsDead() || !oCInformationManager::GetInformationManager().IsDone || ogame->IsOnPause() ) {
      if ( ztimer->factorMotion != 1.0f )
        ztimer->factorMotion = 1.0f;
      return;
    }

    if ( !zinput->KeyToggled( Options::KeyTimeMultiplier ) ) {
      if ( ztimer->factorMotion != Options::TimeMultipliers[multiplierIndex] )
        ztimer->factorMotion = Options::TimeMultipliers[multiplierIndex];
      return;
    }

    multiplierIndex++;
    if ( multiplierIndex < 0 || multiplierIndex >= Options::TimeMultipliers.GetNum() )
      multiplierIndex = 0;

    ztimer->factorMotion = Options::TimeMultipliers[multiplierIndex];
  }

  void PlayerStatus::ShowGameTime() {
    if ( !Options::ShowGameTime && !Options::UseTimeMultiplier )
      return;

    zSTRING str = "";
    zCOLOR color = zCOLOR( 255, 255, 255 );

    if ( Options::ShowGameTime ) {
      int day, hour, min;
      ogame->GetTime( day, hour, min );
      str = (min > 9) ? Z hour + ":" + Z min : Z hour + ":0" + Z min;
    }

    if ( ztimer->factorMotion != 1.0f ) {
      color = zCOLOR( 255, 0, 0 );
      if ( str.Length() ) str = str + " ";
      str = str + "x" + Z ztimer->factorMotion;
    }

    if ( !str.Length() )
      return;

    new IconInfo( screen->FontY(), screen->FontY() * 2.5, color, "ICON_CLOCK", str );
  }

  void PlayerStatus::StatusBars() {
    if ( !hpBar )
      hpBar = new StatusBar( ogame->hpBar );

    if ( !manaBar )
      manaBar = new StatusBar( ogame->manaBar );

    if ( !focusBar )
      focusBar = new StatusBar( ogame->focusBar );

    hpBar->Loop();
    manaBar->Loop();
    focusBar->Loop();
  }

  void PlayerStatus::Clear() {
    focusColor.Clear();
    debugHelper.Clear();

    if ( hpBar )
      hpBar->Clear();

    if ( manaBar )
      manaBar->Clear();

    if ( focusBar )
      focusBar->Clear();
  }

  void PlayerStatus::Loop() {
    if ( !ogame || !player )
      return;

    if ( quickSave->isSaving ) {
      Clear();
      return;
    }

    debugHelper.Loop();
    focusColor.Loop();
    StatusBars();
    FactorMotion();
    ShowGameTime();
  }
}