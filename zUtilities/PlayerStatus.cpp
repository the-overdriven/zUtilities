// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
  HOOK Ivk_CallOnStateFunc_Union PATCH( &oCMobInter::CallOnStateFunc, &oCMobInter::CallOnStateFunc_Union );
  void oCMobInter::CallOnStateFunc_Union( oCNpc* npc, int a1 ) {
    THISCALL( Ivk_CallOnStateFunc_Union )(npc, a1);

    if ( npc == player )
      playerStatus.TryAddStateFunc( this );
  }

  HOOK Ivk_GetStateEffectFunc_Union PATCH( &oCItem::GetStateEffectFunc, &oCItem::GetStateEffectFunc_Union );
  int oCItem::GetStateEffectFunc_Union( int a1 ) {
    int result = THISCALL( Ivk_GetStateEffectFunc_Union )(a1);
    playerStatus.stateFuncItem = (a1 == 0) ? this : nullptr;
    return result;
  }

  HOOK Ivk_EV_UseItemToState_Union PATCH( &oCNpc::EV_UseItemToState, &oCNpc::EV_UseItemToState_Union );
  int oCNpc::EV_UseItemToState_Union( oCMsgManipulate* msg ) {
    int result = THISCALL( Ivk_EV_UseItemToState_Union )(msg);

    if ( this == player && msg && msg->targetVob && playerStatus.stateFuncItem && playerStatus.stateFuncItem == msg->targetVob ) {
      playerStatus.TryAddStateFunc( msg->targetVob );
      playerStatus.stateFuncItem = nullptr;
    }

    return result;
  }

  bool PlayerStatus::KnowStateFunc( zCVob* vob ) {
    if ( !vob )
      return false;

    if ( oCItem* item = vob->CastTo<oCItem>() ) {
      if ( stateFuncItems.IsInList( item->GetInstanceName() ) )
        return true;

      int index = item->GetStateFunc();
      if ( index == Invalid )
        return false;

      for ( int i = 0; i < interStateFuncs.GetNumInList(); i++ ) {
        int idx = parser->GetIndex( interStateFuncs[i] + "_s1" );
        if ( idx == index )
          return true;
      }
    }
    else if ( oCMobInter* inter = vob->CastTo<oCMobInter>() ) {
      if ( interStateFuncs.IsInList( inter->onStateFuncName ) )
        return true;

      int index = parser->GetIndex( inter->onStateFuncName + "_s1" );
      if ( index == Invalid )
        return false;

      for ( int i = 0; i < stateFuncItems.GetNumInList(); i++ ) {
        oCItem* itm = new oCItem( stateFuncItems[i], 1 );
        if ( !itm ) continue;
        int idx = itm->GetStateFunc();
        itm->Release();
        if ( idx == index )
          return true;
      }
    }

    return false;
  }

  void PlayerStatus::TryAddStateFunc( zCVob* vob ) {
    if ( !vob )
      return;

    if ( KnowStateFunc( vob ) )
      return;

    if ( oCItem* item = vob->CastTo<oCItem>() )
      stateFuncItems.Insert( item->GetInstanceName() );
    else if ( oCMobInter* inter = vob->CastTo<oCMobInter>() )
      interStateFuncs.Insert( inter->onStateFuncName );
  }

  void PlayerStatus::Archive() {
    int slotID = SaveLoadGameInfo.slotID;
    if ( slotID < 0 )
      return;

    zCArchiver* ar = zarcFactory->CreateArchiverWrite( Z GetArchivePath( PLUGIN_NAME ), zARC_MODE_ASCII, 0, 0 );
    if ( !ar )
      return;

    ar->WriteInt( "interStateFuncsCount", interStateFuncs.GetNum() );
    for ( uint i = 0; i < interStateFuncs.GetNum(); i++ )
      ar->WriteString( "interStateFuncs", interStateFuncs[i] );

    ar->WriteInt( "stateFuncItemsCount", stateFuncItems.GetNum() );
    for ( uint i = 0; i < stateFuncItems.GetNum(); i++ )
      ar->WriteString( "stateFuncItems", stateFuncItems[i] );

    ar->Close();
    ar->Release();
  }

  void PlayerStatus::Unarchive() {
    int slotID = SaveLoadGameInfo.slotID;
    if ( slotID < 0 )
      return;

    interStateFuncs.EmptyList();
    stateFuncItems.EmptyList();

    zCArchiver* ar = zarcFactory->CreateArchiverRead( Z GetArchivePath( PLUGIN_NAME ), 0 );
    if ( !ar )
      return;

    int interStateFuncsCount;
    ar->ReadInt( "interStateFuncsCount", interStateFuncsCount );
    for ( int i = 0; i < interStateFuncsCount; i++ ) {
      zSTRING str;
      ar->ReadString( "interStateFuncs", str );
      interStateFuncs.Insert( str );
    };

    int stateFuncItemsCount;
    ar->ReadInt( "stateFuncItemsCount", stateFuncItemsCount );
    for ( int i = 0; i < stateFuncItemsCount; i++ ) {
      zSTRING str;
      ar->ReadString( "stateFuncItems", str );
      stateFuncItems.Insert( str );
    };

    ar->Close();
    ar->Release();
  }

#if ENGINE < Engine_G2
  HOOK Hook_zCAICamera_CheckKeys PATCH( &zCAICamera::CheckKeys, &zCAICamera::CheckKeys_Union );
  void zCAICamera::CheckKeys_Union() {
    if ( !Options::UseTimeMultiplier || ztimer->factorMotion == 1.0f ) {
      THISCALL( Hook_zCAICamera_CheckKeys )();
      return;
    }

    float frameTimeOld = ztimer->frameTimeFloat;
    float motionFactorOld = ztimer->factorMotion;
    ztimer->frameTimeFloat = frameTimeOld / motionFactorOld;
    ztimer->factorMotion = 1.0f;
    THISCALL( Hook_zCAICamera_CheckKeys )();
    ztimer->frameTimeFloat = frameTimeOld;
    ztimer->factorMotion = motionFactorOld;
  }

  HOOK Hook_oCAIHuman_PC_Turnings PATCH( &oCAIHuman::PC_Turnings, &oCAIHuman::PC_Turnings_Union );
  void oCAIHuman::PC_Turnings_Union( int forceRotation ) {
    if ( !Options::UseTimeMultiplier || ztimer->factorMotion == 1.0f || Pressed( GAME_LEFT ) || Pressed( GAME_RIGHT ) ) {
      THISCALL( Hook_oCAIHuman_PC_Turnings )(forceRotation);
      return;
    }

    float frameTimeOld = ztimer->frameTimeFloat;
    float motionFactorOld = ztimer->factorMotion;
    ztimer->frameTimeFloat = frameTimeOld / motionFactorOld;
    ztimer->factorMotion = 1.0f;
    THISCALL( Hook_oCAIHuman_PC_Turnings )(forceRotation);
    ztimer->frameTimeFloat = frameTimeOld;
    ztimer->factorMotion = motionFactorOld;
  };
#endif

  void PlayerStatus::ResetTimeMultiplier() {
    if ( !ztimer )
      return;

    ztimer->factorMotion = 1.0f;
  }

  void PlayerStatus::FactorMotion() {
    if ( !Options::UseTimeMultiplier )
      return;

    if ( !Options::TimeMultipliers.GetNum() || playerHelper.IsConUp() )
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

    if ( Options::UseTimeMultiplier && ztimer->factorMotion != 1.0f ) {
      color = zCOLOR( 255, 0, 0 );
      if ( str.Length() ) str = str + " ";
      str = str + "x" + Z ztimer->factorMotion;
    }

    if ( !str.Length() )
      return;

    zSTRING texture = "ICON_CLOCK"; // https://game-icons.net/1x1/lorc/empty-hourglass.html
    new IconInfo( screen->FontY(), screen->FontY() * 2.5, color, texture, str );
  }

  void PlayerStatus::StatusBars() {
    if ( !hpBar )
      hpBar = new StatusBar( ogame->hpBar );

    if ( !manaBar )
      manaBar = new StatusBar( ogame->manaBar );

    if ( !focusBar )
      focusBar = new StatusBar( ogame->focusBar );

    if ( !swimBar )
      swimBar = new StatusBar( ogame->swimBar );

    hpBar->Loop();
    manaBar->Loop();
    focusBar->Loop();
    swimBar->Loop();
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

    if ( swimBar )
      swimBar->Clear();
  }

  void PlayerStatus::Loop() {
    if ( !ogame || !player )
      return;

    if ( quickSave->IsBusy() ) {
      Clear();
      return;
    }

    debugHelper.Loop();
    focusColor.Loop();
    StatusBars();
    FactorMotion();
    ShowGameTime();
    HandleMunitionLoop();
    RenderSelectedItem();
  }
}