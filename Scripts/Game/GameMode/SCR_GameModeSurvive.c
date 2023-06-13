//------------------------------------------------------------------------------------------------
class SCR_GameModeAISurviveClass: SCR_BaseGameModeClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_GameModeAISurvive : SCR_BaseGameMode
{
	[Attribute("10", UIWidgets.EditBox, "How long between clearing the last wave and a new one")]
	float m_TimeoutBetweenWaves;

	[Attribute("4", UIWidgets.EditBox, "How many groups spawned each wave")]
	int m_InitialGroupsCount;

	[Attribute("1", UIWidgets.EditBox, "How many new groups should spawn each round")]
	int m_AddedGroupsPerRound;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "What group type should spawn")]
	ResourceName m_GroupType;
	
	[Attribute("USSR", UIWidgets.EditBox, "Enemy faction key")]
	protected FactionKey m_sEnemyFactionKey;
	
	[Attribute("US", UIWidgets.EditBox, "Friendly faction key")]
	protected FactionKey m_sFriendlyFactionKey;
	
	int m_iGroupsToSpawn;
	int m_iRoundNumber;

	Widget m_wRoot;
	TextWidget m_wText;
	ref array<IEntity> m_aEnemySoldiers;
	ref array<SCR_AIGroup> m_aGroups;
	ref array<SCR_SpawnPoint> m_aEnemySpawnPoints;
	ref array<SCR_SpawnPoint> m_aPlayerSpawnPoints;
	float m_fCurrentTimeout;
	AIWaypoint m_AttackWP;
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		if (m_fCurrentTimeout > 0)
		{
			m_fCurrentTimeout -= timeSlice;
			if (m_fCurrentTimeout < 0)
			{
				m_fCurrentTimeout = 0;
				m_iRoundNumber++;
				if (m_iRoundNumber > 0)
				{
					SpawnEnemies(m_iRoundNumber);
					Print("Sending next wave of enemies.");
				}
			}
		}
		int z = 0;
		int m_alivegroups = 0;
		bool Dead = false;
		foreach( SCR_AIGroup groups : m_aGroups)
		{
			if(!groups)
			{
			}
			else
			{
				m_alivegroups++;
			}
			z++;
		}
		if(m_alivegroups == 0)
		{
			Dead = true;
		}
		if(Dead) 
		{
			Print("All Groups Dead");
			SCR_HintManagerComponent.GetInstance().ShowCustomHint("Clear", "Mission", 10.0);
			m_fCurrentTimeout = 0;
			m_iRoundNumber++;
			if (m_iRoundNumber > 0)
			{
				SpawnEnemies(m_iRoundNumber);
				Print("Sending next wave of enemies.");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void SpawnEnemies(int round)
	{
		foreach(SCR_AIGroup groups : m_aGroups)
		{
			Print(m_aGroups.Count());
		}
		if (m_aEnemySpawnPoints.Count() == 0)
			return;
		Print("Spawning");
		// Clean up groups
		for (int i = m_aGroups.Count() - 1; i >= 0; i--)
		{
			Print(m_aGroups[i]);
			delete m_aGroups[i];
		}
		m_aGroups.Clear();
		m_aEnemySoldiers.Clear();
		
		m_iGroupsToSpawn = m_InitialGroupsCount + (round - 1) * m_AddedGroupsPerRound;
		
		for (int i = 0; i < m_iGroupsToSpawn; i++)
		{
			Print("Generate Agent.");
			RandomGenerator generator = new RandomGenerator;
			generator.SetSeed(Math.RandomInt(0,100));

			SCR_SpawnPoint spawnPoint = m_aEnemySpawnPoints.GetRandomElement();
			if (!spawnPoint)
				return;
			
			vector position = generator.GenerateRandomPointInRadius(0, 2, spawnPoint.GetOrigin());
			position[1] = spawnPoint.GetOrigin()[1];
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = position;
			
			Resource res = Resource.Load(m_GroupType);
			SCR_AIGroup newGrp = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
			m_aGroups.Insert(newGrp);
			array<AIAgent> agents = new array<AIAgent>;
			Print(newGrp.GetAgents(agents));
			Print("Before Foreach");
			foreach (AIAgent agent : agents)
			{
				if (agent)
				{
					m_aEnemySoldiers.Insert(agent.GetControlledEntity());
				}
			}
			//Define checkpoiint
			if (m_AttackWP)
			{
				newGrp.AddWaypointToGroup(m_AttackWP);
			}
		}
		//print(m_aEnemySoldiers.Count());
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleOnCharacterDeath(notnull CharacterControllerComponent characterController, IEntity instigator)
	{
		super.HandleOnCharacterDeath(characterController, instigator);
		
		IEntity victimEntity = characterController.GetCharacter();
		m_aEnemySoldiers.RemoveItemOrdered(victimEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_fCurrentTimeout = m_TimeoutBetweenWaves;

		SCR_HintManagerComponent.GetInstance().ShowCustomHint("Spawning", "Mission01", 10.0);
		
		BaseWorld world = owner.GetWorld();
		m_AttackWP = AIWaypoint.Cast(world.FindEntityByName("WP1"));
		m_aEnemySoldiers = new ref array<IEntity>;
		m_aGroups = new ref array<SCR_AIGroup>;
		m_aPlayerSpawnPoints = new ref array<SCR_SpawnPoint>;
		m_aEnemySpawnPoints = new ref array<SCR_SpawnPoint>;
		array<SCR_SpawnPoint> spawnPoints = new array<SCR_SpawnPoint>;
		spawnPoints = SCR_SpawnPoint.GetSpawnPoints();

		for (int i = spawnPoints.Count() - 1; i >= 0; i--)
		{
			SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(spawnPoints[i]);
			if (spawnPoint)
			{
				FactionKey faction = spawnPoint.GetFactionKey();
				if (faction == m_sEnemyFactionKey)
					m_aEnemySpawnPoints.Insert(spawnPoint);
				
				if (faction == m_sFriendlyFactionKey)
					m_aPlayerSpawnPoints.Insert(spawnPoint);
			}
		}
	}
};

