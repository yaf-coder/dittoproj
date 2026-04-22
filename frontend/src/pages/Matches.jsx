import { useEffect, useState } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '../api/client'

const RESTAURANTS = [
  { name: "L'Orcio",        url: 'http://www.lorcio.com/',         cuisine: 'Italian'          },
  { name: 'Olea',           url: 'http://www.oleanewhaven.com/',   cuisine: 'Mediterranean'    },
  { name: 'Shell & Bones',  url: 'http://www.shellandbones.com/',  cuisine: 'Seafood & Grill'  },
]

function formatMatchedWhen(createdAt) {
  const now  = Date.now() / 1000
  const diff = now - createdAt
  if (diff < 60)       return 'Just now'
  if (diff < 3600)     return `${Math.floor(diff / 60)}m ago`
  if (diff < 86400)    return `${Math.floor(diff / 3600)}h ago`
  if (diff < 604800)   return `${Math.floor(diff / 86400)}d ago`
  return new Date(createdAt * 1000).toLocaleDateString()
}

function MatchDetail({ match, onBack }) {
  return (
    <motion.div
      initial={{ opacity: 0, x: 24 }}
      animate={{ opacity: 1, x: 0 }}
      exit={{ opacity: 0, x: 24 }}
      transition={{ duration: 0.25, ease: [0.25, 0.46, 0.45, 0.94] }}
      className="flex-1 flex flex-col overflow-y-auto"
    >
      {/* Header */}
      <div className="px-4 pt-3 pb-2 shrink-0 flex items-center gap-2">
        <button
          onClick={onBack}
          className="w-9 h-9 rounded-full flex items-center justify-center active:bg-apple-gray-2 transition-colors"
        >
          <svg width="22" height="22" viewBox="0 0 24 24" fill="none">
            <path d="M15 18l-6-6 6-6" stroke="#2D4A6A" strokeWidth="2.2"
              strokeLinecap="round" strokeLinejoin="round"/>
          </svg>
        </button>
        <span className="text-[17px] text-apple-blue font-cursive">Matches</span>
      </div>

      {/* Profile header */}
      <div className="px-6 pt-6 pb-8 flex flex-col items-center text-center">
        <div className="w-28 h-28 rounded-full overflow-hidden bg-apple-gray mb-4 shadow-apple-sm">
          {match.user.picture ? (
            <img src={match.user.picture} alt={match.user.name} className="w-full h-full object-cover" />
          ) : (
            <div className="w-full h-full flex items-center justify-center">
              <span className="text-4xl font-bold text-apple-sub/40">{match.user.name?.[0]}</span>
            </div>
          )}
        </div>
        <h1 className="text-[26px] font-bold tracking-tight2 text-apple-text">
          {match.user.name}{match.user.age ? `, ${match.user.age}` : ''}
        </h1>
        <p className="text-apple-sub text-[14px] mt-1">
          Matched {formatMatchedWhen(match.created_at)}
        </p>
      </div>

      {/* Restaurants */}
      <div className="px-4 pb-10">
        <p className="px-2 mb-2 text-[13px] font-semibold text-apple-sub uppercase tracking-wide">
          Restaurants {match.user.name?.split(' ')[0]} likes
        </p>
        <div className="bg-white rounded-3xl shadow-apple-sm overflow-hidden">
          {RESTAURANTS.map((r, i) => (
            <a
              key={r.url}
              href={r.url}
              target="_blank"
              rel="noopener noreferrer"
              className={[
                'flex items-center gap-4 px-5 py-4',
                i < RESTAURANTS.length - 1 ? 'border-b border-apple-gray-2' : '',
                'active:bg-apple-gray-2 transition-colors',
              ].join(' ')}
            >
              <div className="w-11 h-11 rounded-xl bg-apple-blue/10 flex items-center justify-center shrink-0">
                <svg width="22" height="22" viewBox="0 0 24 24" fill="none">
                  <path d="M7 3v9a3 3 0 0 0 3 3v6M7 3l0 6M10 3l0 6M17 3v18M17 13c-1.5 0-3-1-3-3V5c0-1 1-2 3-2"
                    stroke="#2D4A6A" strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round"/>
                </svg>
              </div>
              <div className="flex-1 min-w-0">
                <p className="text-[16px] font-semibold text-apple-text truncate">{r.name}</p>
                <p className="text-[13px] text-apple-sub truncate">{r.cuisine}</p>
              </div>
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none">
                <path d="M7 17L17 7M17 7H9M17 7v8" stroke="#C7C7CC" strokeWidth="2"
                  strokeLinecap="round" strokeLinejoin="round"/>
              </svg>
            </a>
          ))}
        </div>
      </div>
    </motion.div>
  )
}

export default function Matches() {
  const [matches, setMatches] = useState([])
  const [loading, setLoading] = useState(true)
  const [selected, setSelected] = useState(null)

  useEffect(() => {
    api.getMatches()
      .then(setMatches)
      .finally(() => setLoading(false))
  }, [])

  if (loading) {
    return (
      <div className="flex-1 flex items-center justify-center">
        <div className="w-8 h-8 border-[2.5px] border-apple-blue border-t-transparent rounded-full animate-spin" />
      </div>
    )
  }

  return (
    <AnimatePresence mode="wait">
      {selected ? (
        <MatchDetail
          key="detail"
          match={selected}
          onBack={() => setSelected(null)}
        />
      ) : (
        <motion.div
          key="list"
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          transition={{ duration: 0.2 }}
          className="flex-1 flex flex-col overflow-y-auto"
        >
          <div className="px-6 pt-5 pb-4 shrink-0">
            <h1 className="text-lg font-cursive text-apple-text">Matches</h1>
            <p className="text-[15px] text-apple-sub mt-0.5">
              {matches.length === 0
                ? 'No matches yet — keep swiping.'
                : `${matches.length} mutual ${matches.length === 1 ? 'match' : 'matches'}`}
            </p>
          </div>

          {matches.length === 0 ? (
            <div className="flex-1 flex flex-col items-center justify-center gap-3 px-8 pb-16">
              <div className="w-20 h-20 rounded-full bg-apple-gray flex items-center justify-center">
                <svg width="36" height="36" viewBox="0 0 24 24" fill="none">
                  <path d="M12 21C12 21 3 15 3 8.5A5 5 0 0 1 12 6a5 5 0 0 1 9 2.5C21 15 12 21 12 21z"
                    stroke="#8D8FA8" strokeWidth="1.8" strokeLinejoin="round"/>
                </svg>
              </div>
              <p className="text-apple-sub/60 text-[14px] text-center">
                When someone you've liked likes you back, they'll show up here.
              </p>
            </div>
          ) : (
            <div className="px-4 pb-10 flex flex-col gap-2">
              {matches.map((m, i) => (
                <motion.button
                  key={m.match_id}
                  initial={{ opacity: 0, y: 12 }}
                  animate={{ opacity: 1, y: 0 }}
                  transition={{ duration: 0.3, delay: i * 0.04 }}
                  onClick={() => setSelected(m)}
                  className="bg-white rounded-3xl shadow-apple-sm px-4 py-3 flex items-center gap-4 text-left active:bg-apple-gray-2 transition-colors"
                >
                  <div className="w-14 h-14 rounded-full overflow-hidden bg-apple-gray shrink-0">
                    {m.user.picture ? (
                      <img src={m.user.picture} alt={m.user.name} className="w-full h-full object-cover" />
                    ) : (
                      <div className="w-full h-full flex items-center justify-center">
                        <span className="text-xl font-bold text-apple-sub/40">
                          {m.user.name?.[0]}
                        </span>
                      </div>
                    )}
                  </div>
                  <div className="flex-1 min-w-0">
                    <p className="text-[17px] font-semibold text-apple-text truncate">
                      {m.user.name}{m.user.age ? `, ${m.user.age}` : ''}
                    </p>
                    <p className="text-[13px] text-apple-sub">
                      Matched {formatMatchedWhen(m.created_at)}
                    </p>
                  </div>
                  <svg width="16" height="16" viewBox="0 0 24 24" fill="none">
                    <path d="M9 18l6-6-6-6" stroke="#C7C7CC" strokeWidth="2"
                      strokeLinecap="round" strokeLinejoin="round"/>
                  </svg>
                </motion.button>
              ))}
            </div>
          )}
        </motion.div>
      )}
    </AnimatePresence>
  )
}
